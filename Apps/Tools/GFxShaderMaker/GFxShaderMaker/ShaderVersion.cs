using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text.RegularExpressions;
using System.Xml;

namespace GFxShaderMaker;

public abstract class ShaderVersion
{
	private class PipelineSrcList
	{
		public uint Index;

		public List<ShaderLinkedSource> Sources;

		public PipelineSrcList(uint index, List<ShaderLinkedSource> srcs)
		{
			Index = index;
			Sources = srcs;
		}
	}

	protected List<ShaderPipeline> SupportedPipelines = new List<ShaderPipeline>();

	public List<ShaderSource> ShaderSources;

	public List<ShaderLinkedSource> LinkedSources;

	public Dictionary<int, ShaderLinkedSource> LinkedSourceDuplicates;

	public Dictionary<int, List<ShaderLinkedSource>> LinkedSourceUniqueDescs;

	public string ID { get; private set; }

	public ShaderPlatform Platform { get; private set; }

	public IEnumerable<ShaderPipeline> Pipelines => SupportedPipelines;

	public virtual List<string> UnsupportedFlags => new List<string>();

	public virtual bool RequireShaderCombos => false;

	public abstract string SourceExtension { get; }

	protected virtual string BatchRoundOffString => "+ 0.1f";

	protected virtual string BatchIndexType => "float";

	public virtual uint MaxBatchCount => 24u;

	public string SourceDirectory => Platform.PlatformObjDirectory;

	public List<ShaderVariable> UniqueUniformList
	{
		get
		{
			List<ShaderVariable> list = new List<ShaderVariable>();
			foreach (ShaderLinkedSource linkedSource in LinkedSources)
			{
				foreach (ShaderVariable item in linkedSource.VariableList.FindAll((ShaderVariable var) => var.VarType == ShaderVariable.VariableType.Variable_Uniform || var.VarType == ShaderVariable.VariableType.Variable_VirtualUniform))
				{
					list.Add(item);
				}
			}
			List<ShaderVariable> list2 = list.Distinct(new ShaderVariableCompareIDs()).ToList();
			list2.Sort();
			return list2;
		}
	}

	public static string SubexprRegex => "[^\\(\\)]*(((?'Open'\\()[^\\(\\)]*)+((?'Close-Open'\\))[^\\(\\)]*)+)*(?(Open)(?!))";

	public ShaderVersion(ShaderPlatform platform, string id)
	{
		Platform = platform;
		ID = id;
		if (SupportedPipelines.Find((ShaderPipeline p) => p.Type == ShaderPipeline.PipelineType.Vertex) == null)
		{
			SupportedPipelines.Add(new ShaderPipeline(ShaderPipeline.PipelineType.Vertex));
		}
		if (SupportedPipelines.Find((ShaderPipeline p) => p.Type == ShaderPipeline.PipelineType.Fragment) == null)
		{
			SupportedPipelines.Add(new ShaderPipeline(ShaderPipeline.PipelineType.Fragment));
		}
	}

	public abstract string GetVariableUniformRegisterType(ShaderVariable var);

	public void ReadFromXml(XmlElement root)
	{
		ShaderSources = new List<ShaderSource>();
		foreach (XmlElement item in root.GetElementsByTagName("ShaderSource"))
		{
			ShaderSource shaderSource = new ShaderSource();
			shaderSource.ReadFromXml(item);
			ShaderSources.Add(shaderSource);
		}
		Dictionary<KeyValuePair<string, ShaderPipeline.PipelineType>, List<ShaderSource>> dictionary = new Dictionary<KeyValuePair<string, ShaderPipeline.PipelineType>, List<ShaderSource>>();
		foreach (ShaderSource shaderSource3 in ShaderSources)
		{
			KeyValuePair<string, ShaderPipeline.PipelineType> key = new KeyValuePair<string, ShaderPipeline.PipelineType>(shaderSource3.ID, shaderSource3.PipelineType);
			if (!dictionary.TryGetValue(key, out var value))
			{
				value = new List<ShaderSource>();
				dictionary.Add(key, value);
			}
			value.Add(shaderSource3);
		}
		ShaderSources.Clear();
		foreach (KeyValuePair<string, ShaderPipeline.PipelineType> key2 in dictionary.Keys)
		{
			List<ShaderSource> list = dictionary[key2];
			ShaderSource shaderSource2 = list.Find((ShaderSource s) => s.Versions.Contains(ID));
			if (shaderSource2 == null)
			{
				shaderSource2 = list.Find((ShaderSource s) => s.Platforms.Contains(Platform.PlatformName));
			}
			if (shaderSource2 == null)
			{
				shaderSource2 = list.Find((ShaderSource s) => s.Platforms.Count == 0 && s.Versions.Count == 0);
			}
			if (shaderSource2 != null)
			{
				ShaderSources.Add(shaderSource2);
			}
		}
	}

	public virtual void CreatePermutations()
	{
		List<ShaderLinkedSource> list = new List<ShaderLinkedSource>();
		foreach (ShaderGroup shaderGroup in Platform.ShaderGroups)
		{
			list.AddRange(shaderGroup.LinkShaderSources(ShaderSources, this));
		}
		LinkedSources = list;
		foreach (ShaderLinkedSource linkedSource in LinkedSources)
		{
			foreach (string postFunction in linkedSource.PostFunctions)
			{
				GetType().GetMethod("PostLink_" + postFunction)?.Invoke(this, new object[1] { linkedSource });
			}
			AssignRegisters(linkedSource);
			linkedSource.SourceCode = CreateFinalSource(linkedSource);
		}
		LinkedSourceDuplicates = HashDuplicatesSources();
		LinkedSourceUniqueDescs = CreateUniqueDescriptorMap();
	}

	private Dictionary<int, List<ShaderLinkedSource>> CreateUniqueDescriptorMap()
	{
		Dictionary<int, List<ShaderLinkedSource>> dictionary = new Dictionary<int, List<ShaderLinkedSource>>();
		foreach (ShaderLinkedSource linkedSource in LinkedSources)
		{
			int hashCode = linkedSource.GetHashCode();
			if (!dictionary.TryGetValue(hashCode, out var value))
			{
				value = new List<ShaderLinkedSource>();
				dictionary.Add(hashCode, value);
			}
			value.Add(linkedSource);
		}
		return dictionary;
	}

	private Dictionary<int, ShaderLinkedSource> HashDuplicatesSources()
	{
		Dictionary<int, List<ShaderLinkedSource>> dictionary = new Dictionary<int, List<ShaderLinkedSource>>();
		Dictionary<string, int> dictionary2 = new Dictionary<string, int>();
		foreach (ShaderLinkedSource linkedSource in LinkedSources)
		{
			int hashCode = linkedSource.SourceCode.GetHashCode();
			int value = 0;
			if (dictionary2.TryGetValue(linkedSource.ID, out value))
			{
				throw new Exception("Error: Two shader permutations have the same name (id=" + linkedSource.ID + ").\n");
			}
			dictionary2.Add(linkedSource.ID, hashCode);
			if (dictionary.ContainsKey(hashCode))
			{
				dictionary.TryGetValue(hashCode, out var value2);
				value2.Add(linkedSource);
				linkedSource.SourceCodeDuplicateID = value2[0].ID;
			}
			else
			{
				List<ShaderLinkedSource> list = new List<ShaderLinkedSource>();
				list.Add(linkedSource);
				dictionary.Add(hashCode, list);
				linkedSource.SourceCodeDuplicateID = linkedSource.ID;
			}
		}
		ShaderPipeline pipeline;
		foreach (ShaderPipeline pipeline2 in Pipelines)
		{
			pipeline = pipeline2;
			uint num = 1u;
			foreach (List<ShaderLinkedSource> value3 in dictionary.Values)
			{
				bool flag = false;
				foreach (ShaderLinkedSource item in value3.FindAll((ShaderLinkedSource s) => s.Pipeline == pipeline))
				{
					item.ShaderIndex = num;
					flag = true;
				}
				if (flag)
				{
					num++;
				}
			}
		}
		List<PipelineSrcList> list2 = new List<PipelineSrcList>();
		uint num2 = 0u;
		foreach (ShaderGroup shaderGroup in Platform.ShaderGroups)
		{
			foreach (ShaderPermutation permutation in shaderGroup.Permutations)
			{
				string permutationName = permutation.GetPermutationName();
				List<ShaderLinkedSource> list3 = new List<ShaderLinkedSource>();
				List<ShaderLinkedSource> hashedPipelineSrcs = new List<ShaderLinkedSource>();
				foreach (ShaderPipeline pipeline3 in Pipelines)
				{
					string name = pipeline3.Name;
					char c = name[0];
					string linkedSrcName = c + permutationName;
					foreach (List<ShaderLinkedSource> value4 in dictionary.Values)
					{
						ShaderLinkedSource shaderLinkedSource = value4.Find((ShaderLinkedSource s) => s.ID == linkedSrcName);
						if (shaderLinkedSource != null)
						{
							hashedPipelineSrcs.Add(value4.First());
							list3.Add(shaderLinkedSource);
							break;
						}
					}
				}
				PipelineSrcList pipelineSrcList = list2.Find((PipelineSrcList psl) => psl.Sources.SequenceEqual(hashedPipelineSrcs));
				if (pipelineSrcList == null)
				{
					pipelineSrcList = new PipelineSrcList(num2++, hashedPipelineSrcs);
					list2.Add(pipelineSrcList);
				}
				foreach (ShaderLinkedSource item2 in list3)
				{
					item2.ShaderComboIndex = pipelineSrcList.Index;
				}
			}
		}
		Dictionary<int, ShaderLinkedSource> dictionary3 = new Dictionary<int, ShaderLinkedSource>();
		foreach (List<ShaderLinkedSource> value5 in dictionary.Values)
		{
			dictionary3.Add(value5[0].SourceCode.GetHashCode(), value5[0]);
		}
		return dictionary3;
	}

	public virtual void AssignRegisters(ShaderLinkedSource src)
	{
		src.VariableList.Sort();
		Dictionary<string, uint> dictionary = new Dictionary<string, uint>();
		Dictionary<string, uint> dictionary2 = new Dictionary<string, uint>();
		foreach (ShaderVariable variable in src.VariableList)
		{
			switch (variable.VarType)
			{
			case ShaderVariable.VariableType.Variable_Attribute:
			case ShaderVariable.VariableType.Variable_Varying:
			case ShaderVariable.VariableType.Variable_FragOut:
			case ShaderVariable.VariableType.Variable_VirtualAttribute:
			{
				string text = "";
				text = variable.VarType switch
				{
					ShaderVariable.VariableType.Variable_Varying => (src.Pipeline.Type == ShaderPipeline.PipelineType.Vertex) ? "O" : "I", 
					ShaderVariable.VariableType.Variable_FragOut => "O", 
					_ => "I", 
				};
				uint value2 = 0u;
				if (dictionary.TryGetValue(text + variable.Semantic, out value2))
				{
					value2++;
				}
				dictionary[text + variable.Semantic] = value2;
				variable.Semantic += value2;
				break;
			}
			case ShaderVariable.VariableType.Variable_Uniform:
			{
				string variableUniformRegisterType = GetVariableUniformRegisterType(variable);
				uint value = 0u;
				dictionary2.TryGetValue(variableUniformRegisterType, out value);
				variable.BaseRegister = value;
				value += variable.RegisterCount;
				dictionary2[variableUniformRegisterType] = value;
				break;
			}
			}
		}
	}

	public virtual void PostLink_Batch(ShaderLinkedSource linkedSrc)
	{
		if (linkedSrc.Pipeline.Type != ShaderPipeline.PipelineType.Vertex)
		{
			return;
		}
		uint num = 0u;
		uint num2 = 0u;
		foreach (ShaderVariable item in linkedSrc.VariableList.FindAll((ShaderVariable var) => var.VarType == ShaderVariable.VariableType.Variable_Uniform))
		{
			if (item.ElementCount == 16)
			{
				item.Semantic = "vfmuniforms:" + num2;
				num2++;
			}
			else
			{
				item.Semantic = "vfuniforms:" + num;
				num += item.RegisterCount;
			}
		}
		foreach (ShaderVariable item2 in linkedSrc.VariableList.FindAll((ShaderVariable var) => var.VarType == ShaderVariable.VariableType.Variable_Uniform))
		{
			item2.VarType = ShaderVariable.VariableType.Variable_VirtualUniform;
			string[] array = item2.Semantic.Split(":".ToCharArray());
			uint num3 = ((array[0] == "vfuniforms") ? num : num2);
			linkedSrc.SourceCode = Regex.Replace(linkedSrc.SourceCode, "\\b" + item2.ID + "\\b", array[0] + "[vbatch * " + num3 + " + " + array[1].ToString() + BatchRoundOffString + "]");
		}
		linkedSrc.SourceCode = Regex.Replace(linkedSrc.SourceCode, "(vfuniforms\\s*\\[vbatch[^\\]]+)\\]\\s*\\[", "$1 + ");
		if (num2 != 0)
		{
			ShaderVariable shaderVariable = new ShaderVariable();
			shaderVariable.VarType = ShaderVariable.VariableType.Variable_Uniform;
			shaderVariable.ID = "vfmuniforms";
			shaderVariable.Type = "float4x4";
			shaderVariable.Semantic = "";
			shaderVariable.ArraySize = num2 * MaxBatchCount;
			linkedSrc.VariableList.Add(shaderVariable);
		}
		if (num != 0)
		{
			ShaderVariable shaderVariable2 = new ShaderVariable();
			shaderVariable2.VarType = ShaderVariable.VariableType.Variable_Uniform;
			shaderVariable2.ID = "vfuniforms";
			shaderVariable2.Type = "float4";
			shaderVariable2.Semantic = "";
			shaderVariable2.ArraySize = num * MaxBatchCount;
			linkedSrc.VariableList.Add(shaderVariable2);
		}
		if (num2 != 0 || num != 0)
		{
			ShaderVariable shaderVariable3 = new ShaderVariable();
			shaderVariable3.VarType = ShaderVariable.VariableType.Variable_Attribute;
			shaderVariable3.ID = "vbatch";
			shaderVariable3.Type = BatchIndexType;
			shaderVariable3.Semantic = "INSTANCE";
			linkedSrc.VariableList.Add(shaderVariable3);
		}
		ShaderVariable shaderVariable4 = linkedSrc.VariableList.Find((ShaderVariable var) => var.VarType == ShaderVariable.VariableType.Variable_Attribute && var.Semantic.StartsWith("factor", StringComparison.InvariantCultureIgnoreCase));
		ShaderVariable shaderVariable5 = linkedSrc.VariableList.Find((ShaderVariable var) => var.ID == "vbatch");
		if (shaderVariable4 != null && shaderVariable5 != null)
		{
			linkedSrc.VariableList.Remove(shaderVariable5);
			linkedSrc.SourceCode = linkedSrc.SourceCode.Replace(shaderVariable5.ID, shaderVariable4.ID + ".b*255.01f");
		}
	}

	protected void UndoFactorBatchIndexing(ShaderLinkedSource linkedSrc)
	{
		ShaderVariable shaderVariable = linkedSrc.VariableList.Find((ShaderVariable var) => var.ID == "vbatch");
		ShaderVariable shaderVariable2 = linkedSrc.VariableList.Find((ShaderVariable var) => var.VarType == ShaderVariable.VariableType.Variable_Attribute && var.Semantic.StartsWith("factor", StringComparison.InvariantCultureIgnoreCase));
		if (shaderVariable2 != null && shaderVariable == null)
		{
			ShaderVariable shaderVariable3 = new ShaderVariable();
			shaderVariable3.VarType = ShaderVariable.VariableType.Variable_Attribute;
			shaderVariable3.ID = "vbatch";
			shaderVariable3.Type = BatchIndexType;
			shaderVariable3.Semantic = "INSTANCE";
			linkedSrc.VariableList.Add(shaderVariable3);
			linkedSrc.SourceCode = linkedSrc.SourceCode.Replace(shaderVariable2.ID + ".b*255.01f", "vbatch");
		}
	}

	public abstract void PostLink_Instanced(ShaderLinkedSource linkedSrc);

	public abstract string CreateFinalSource(ShaderLinkedSource linkedSrc);

	public virtual void WriteBinaryShaderSource(StreamWriter file)
	{
		throw new NotImplementedException("WriteBinaryShaderSource");
	}

	public virtual void WriteBinaryShaderDataFile()
	{
		throw new NotImplementedException("WriteBinaryShaderDataFile");
	}

	public virtual string GetShaderFilename(ShaderLinkedSource src)
	{
		return ID + "_" + src.ID + SourceExtension;
	}

	public virtual string GetShaderDuplicateFilename(ShaderLinkedSource src)
	{
		return ID + "_" + src.SourceCodeDuplicateID + SourceExtension;
	}

	public virtual string GetSourceCodeContent(ShaderLinkedSource src)
	{
		return src.SourceCode;
	}
}
