using System;
using System.Collections.Generic;
using System.Linq;

namespace GFxShaderMaker;

public class ShaderPermutation : IComparable<ShaderPermutation>
{
	public List<ShaderFeatureFlavor> SpecificFeatures = new List<ShaderFeatureFlavor>();

	public List<ShaderLinkedSource> LinkedSources;

	private uint ShaderTypeValue;

	private bool ShaderTypeSet;

	public uint ShaderType
	{
		get
		{
			if (ShaderTypeSet)
			{
				return ShaderTypeValue;
			}
			throw new Exception("Internal error: ShaderType accessed before being set.");
		}
		private set
		{
			ShaderTypeValue = value;
			ShaderTypeSet = true;
		}
	}

	public void ComputeShaderType(Dictionary<string, uint> flagsDict)
	{
		uint num = 0u;
		foreach (ShaderFeatureFlavor specificFeature in SpecificFeatures)
		{
			if (flagsDict.ContainsKey(specificFeature.ID))
			{
				num += flagsDict[specificFeature.ID];
			}
		}
		ShaderType = num;
	}

	public string GetPermutationName(bool includeHidden = false)
	{
		string text = "";
		foreach (ShaderFeatureFlavor specificFeature in SpecificFeatures)
		{
			if (includeHidden || !specificFeature.Hidden)
			{
				text += specificFeature.ID;
			}
		}
		return text;
	}

	public bool IsValid()
	{
		List<string> list = new List<string>();
		foreach (ShaderFeatureFlavor specificFeature in SpecificFeatures)
		{
			list.AddRange(specificFeature.ExcludeIDs);
		}
		string exclusive;
		foreach (string item in list)
		{
			exclusive = item;
			if (SpecificFeatures.Find((ShaderFeatureFlavor flavor) => flavor.ID == exclusive) != null)
			{
				return false;
			}
		}
		return true;
	}

	public bool IsValid(ShaderVersion ver)
	{
		if (!IsValid())
		{
			return false;
		}
		ShaderFeatureFlavor flavor;
		foreach (ShaderFeatureFlavor specificFeature in SpecificFeatures)
		{
			flavor = specificFeature;
			if (ver.UnsupportedFlags.Find((string uf) => flavor.Flags.Find((string f) => f == uf) != null || flavor.PostLink.Find((string pl) => pl == uf) != null) != null)
			{
				return false;
			}
			if (flavor.IsRestricted(ver))
			{
				return false;
			}
		}
		return true;
	}

	public IList<ShaderLinkedSource> LinkShaderSources(List<ShaderSource> sources, ShaderVersion shaderVersion)
	{
		List<ShaderLinkedSource> list = (LinkedSources = new List<ShaderLinkedSource>());
		List<string> list2 = new List<string>();
		List<ShaderSource> list3 = new List<ShaderSource>();
		List<string> list4 = new List<string>();
		List<string> flags = new List<string>();
		ShaderFeatureFlavor flavor;
		foreach (ShaderFeatureFlavor specificFeature in SpecificFeatures)
		{
			flavor = specificFeature;
			List<ShaderSource> collection = sources.FindAll((ShaderSource src) => src.ID == flavor.ID);
			list3.AddRange(collection);
			list4.AddRange(flavor.PostLink);
			flags.AddRange(flavor.Flags);
			flags.AddRange(flavor.PostLink);
			list2.AddRange(flavor.RequireIDs);
			if (flavor.ID != ShaderFeatureFlavor.EmptyID && list3.Count == 0 && flavor.PostLink.Count == 0 && flavor.RequireIDs.Count == 0 && CommandLineParser.GetOption<int>(CommandLineParser.Options.Verbosity) > 0)
			{
				Console.WriteLine("Warning: " + GetPermutationName() + "." + flavor.ID + " contributes nothing to any pipeline");
			}
		}
		string depSrc;
		foreach (string item in list2.Distinct())
		{
			depSrc = item;
			if (list3.Find((ShaderSource src) => src.ID == depSrc) == null)
			{
				list3.AddRange(sources.FindAll((ShaderSource src) => src.ID == depSrc));
			}
		}
		if (shaderVersion.UnsupportedFlags.Find((string uf) => flags.Find((string f) => f == uf) != null) != null)
		{
			return list;
		}
		ShaderPipeline pipeline;
		foreach (ShaderPipeline pipeline2 in shaderVersion.Pipelines)
		{
			pipeline = pipeline2;
			List<ShaderSource> list5 = list3.FindAll((ShaderSource src) => src.PipelineType == pipeline.Type);
			if (list5.Count == 0)
			{
				continue;
			}
			ShaderLinkedSource shaderLinkedSource = new ShaderLinkedSource();
			list.Add(shaderLinkedSource);
			shaderLinkedSource.Pipeline = pipeline;
			shaderLinkedSource.PostFunctions.AddRange(list4.Distinct());
			shaderLinkedSource.Flags.AddRange(flags.Distinct());
			List<ShaderVariable> list6 = new List<ShaderVariable>();
			foreach (ShaderSource item2 in list5)
			{
				list6.AddRange(item2.Variables);
			}
			list6.Sort();
			IEnumerable<ShaderVariable> enumerable = list6.Distinct(new ShaderVariableCompareIDs());
			string text = "";
			foreach (ShaderSource item3 in list5)
			{
				text = text + item3.CodeOnly + "\n";
			}
			shaderLinkedSource.ID = pipeline.Letter + GetPermutationName();
			shaderLinkedSource.SourceCode = text;
			foreach (ShaderVariable item4 in enumerable)
			{
				shaderLinkedSource.VariableList.Add(item4.Clone() as ShaderVariable);
			}
		}
		return list;
	}

	public int CompareTo(ShaderPermutation other)
	{
		if (other.ShaderType == ShaderType)
		{
			return 0;
		}
		if (other.ShaderType >= ShaderType)
		{
			return -1;
		}
		return 1;
	}
}
