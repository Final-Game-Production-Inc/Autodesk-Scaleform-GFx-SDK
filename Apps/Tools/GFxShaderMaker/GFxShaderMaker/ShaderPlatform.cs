using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading;
using System.Xml;

namespace GFxShaderMaker;

public abstract class ShaderPlatform
{
	protected class CompileThreadData
	{
		public ShaderPlatform This;

		public ShaderVersion SVersion;

		public ShaderLinkedSource Source;

		public string Exe;

		public string ShaderFilename;

		public string CommandLine;

		public string StdOutput;

		public string StdError;

		public int ExitCode;

		public Exception Exception;

		public CompileThreadData(ShaderPlatform p, ShaderVersion v, ShaderLinkedSource s, string f)
		{
			This = p;
			SVersion = v;
			Source = s;
			Exe = f;
			StdOutput = "";
			StdError = "";
			ExitCode = 0;
			Exception = null;
		}
	}

	public enum ShaderBuildEventType
	{
		ShaderBuildEvent_Initialize,
		ShaderBuildEvent_PostShaderDesc,
		ShaderBuildEvent_Finalize
	}

	public List<ShaderGroup> ShaderGroups;

	public Dictionary<string, uint> FeatureFlavorFlags;

	private static Dictionary<int, StringBuilder> launchProcessStdOuts;

	private static Dictionary<int, StringBuilder> launchProcessStdErrs;

	public string PlatformName
	{
		get
		{
			PlatformAttribute platformAttribute = GetPlatformAttribute();
			if (platformAttribute == null)
			{
				return "Unknown";
			}
			return platformAttribute.Name;
		}
	}

	public virtual string PlatformHeaderExtension => ".h";

	public virtual string PlatformSourceExtension => ".cpp";

	public virtual string PlatformBase => PlatformName;

	public virtual string PlatformHeaderFilename => Path.Combine(CommandLineParser.GetOption(CommandLineParser.Options.OutputDirectory), PlatformName + "_ShaderDescs" + PlatformHeaderExtension);

	public virtual string PlatformSourceFilename => Path.Combine(CommandLineParser.GetOption(CommandLineParser.Options.OutputDirectory), PlatformName + "_ShaderDescs" + PlatformSourceExtension);

	public virtual string PlatformBinarySourceFilename => Path.Combine(CommandLineParser.GetOption(CommandLineParser.Options.OutputDirectory), PlatformName + "_ShaderBinary" + PlatformSourceExtension);

	public virtual string PlatformSourceSourceFilename => Path.Combine(CommandLineParser.GetOption(CommandLineParser.Options.OutputDirectory), PlatformName + "_ShaderSource" + PlatformSourceExtension);

	public string PlatformSourceDirectory => Path.Combine(CommandLineParser.GetOption(CommandLineParser.Options.OutputDirectory), "Shaders");

	public virtual string PlatformObjDirectory
	{
		get
		{
			string text = CommandLineParser.GetOption(CommandLineParser.Options.ObjectDirectory);
			if (string.IsNullOrEmpty(text))
			{
				text = Path.Combine(CommandLineParser.GetOption(CommandLineParser.Options.OutputDirectory), "..\\..\\..");
				text = Path.Combine(text, "Obj\\" + PlatformBase + ((PlatformCompiler.Length > 0) ? ("-" + PlatformCompiler) : "") + "\\" + CommandLineParser.GetOption(CommandLineParser.Options.Config));
				text = Path.Combine(text, "Src\\Render\\" + PlatformBase + "\\Shaders");
			}
			text = text.Replace('\\', Path.DirectorySeparatorChar);
			text = text.Replace('/', Path.DirectorySeparatorChar);
			DirectoryInfo directoryInfo = new DirectoryInfo(text);
			return directoryInfo.FullName;
		}
	}

	public virtual string PlatformLibDirectory
	{
		get
		{
			string text = CommandLineParser.GetOption(CommandLineParser.Options.LibraryDirectory);
			if (string.IsNullOrEmpty(text))
			{
				text = Path.Combine(CommandLineParser.GetOption(CommandLineParser.Options.OutputDirectory), "..\\..\\..");
				text = Path.Combine(text, "Lib\\" + PlatformBase + "\\" + PlatformCompiler + "\\" + CommandLineParser.GetOption(CommandLineParser.Options.Config));
			}
			text = text.Replace('\\', Path.DirectorySeparatorChar);
			text = text.Replace('/', Path.DirectorySeparatorChar);
			DirectoryInfo directoryInfo = new DirectoryInfo(text);
			return directoryInfo.FullName;
		}
	}

	public virtual string PlatformBinaryLibrary => Path.Combine(PlatformLibDirectory, "libgfxshaders.a");

	public virtual string PlatformCompiler => "";

	public virtual List<ShaderVersion> PossibleShaderVersions => RequestedShaderVersions;

	public virtual ShaderVersion DefaultShaderVersion => RequestedShaderVersions.First();

	public abstract List<ShaderVersion> RequestedShaderVersions { get; }

	public static List<ShaderPlatform> PlatformList { get; private set; }

	private int MaximumSimultaneousCompileThreads => Environment.ProcessorCount + 1;

	protected List<ShaderVariable> UniqueUniformList
	{
		get
		{
			List<ShaderVariable> list = new List<ShaderVariable>();
			foreach (ShaderVersion possibleShaderVersion in PossibleShaderVersions)
			{
				list.AddRange(possibleShaderVersion.UniqueUniformList);
			}
			List<ShaderVariable> list2 = list.Distinct(new ShaderVariableCompareIDs()).ToList();
			list2.Sort();
			return list2;
		}
	}

	protected List<ShaderPipeline> UsedPipelineList
	{
		get
		{
			List<ShaderPipeline> list = new List<ShaderPipeline>();
			RequestedShaderVersions.ForEach(delegate(ShaderVersion v)
			{
				list.AddRange(v.Pipelines);
			});
			list = list.Distinct(new ShaderPipelineCompareTypes()).ToList();
			return list;
		}
	}

	protected int MaximumVertexAttributes
	{
		get
		{
			int num = 0;
			foreach (ShaderVersion requestedShaderVersion in RequestedShaderVersions)
			{
				foreach (List<ShaderLinkedSource> value in requestedShaderVersion.LinkedSourceUniqueDescs.Values)
				{
					num = Math.Max(num, value.Max((ShaderLinkedSource src) => src.VariableList.FindAll((ShaderVariable v) => v.VarType == ShaderVariable.VariableType.Variable_Attribute || v.VarType == ShaderVariable.VariableType.Variable_VirtualAttribute).Count));
				}
			}
			return num;
		}
	}

	static ShaderPlatform()
	{
		launchProcessStdOuts = new Dictionary<int, StringBuilder>();
		launchProcessStdErrs = new Dictionary<int, StringBuilder>();
		LinkedList<ShaderPlatform> linkedList = new LinkedList<ShaderPlatform>();
		Type[] types = Assembly.GetExecutingAssembly().GetTypes();
		Type[] array = types;
		foreach (Type type in array)
		{
			object[] customAttributes = type.GetCustomAttributes(typeof(PlatformAttribute), inherit: true);
			if (customAttributes.GetLength(0) > 0)
			{
				ShaderPlatform value = (ShaderPlatform)Activator.CreateInstance(type);
				linkedList.AddFirst(value);
			}
		}
		PlatformList = linkedList.ToList();
	}

	public PlatformAttribute GetPlatformAttribute()
	{
		object[] customAttributes = GetType().GetCustomAttributes(typeof(PlatformAttribute), inherit: true);
		if (customAttributes.GetLength(0) > 0)
		{
			return customAttributes[0] as PlatformAttribute;
		}
		return null;
	}

	public void ReadFromXml(XmlElement root)
	{
		ShaderGroups = new List<ShaderGroup>();
		foreach (XmlElement item in root.GetElementsByTagName("ShaderGroup"))
		{
			ShaderGroup shaderGroup = new ShaderGroup();
			shaderGroup.ReadFromXml(item);
			ShaderGroups.Add(shaderGroup);
		}
		foreach (ShaderVersion possibleShaderVersion in PossibleShaderVersions)
		{
			possibleShaderVersion.ReadFromXml(root);
		}
		CreatePermutations();
	}

	private void CreatePermutations()
	{
		DetermineFeatureFlags();
		foreach (ShaderGroup shaderGroup in ShaderGroups)
		{
			shaderGroup.CreatePermutations(FeatureFlavorFlags);
		}
		foreach (ShaderVersion possibleShaderVersion in PossibleShaderVersions)
		{
			possibleShaderVersion.CreatePermutations();
		}
	}

	private void DetermineFeatureFlags()
	{
		FeatureFlavorFlags = new Dictionary<string, uint>();
		Dictionary<string, List<ShaderGroup>> dictionary = new Dictionary<string, List<ShaderGroup>>();
		List<string> list = new List<string>();
		foreach (ShaderGroup shaderGroup in ShaderGroups)
		{
			foreach (ShaderFeature feature in shaderGroup.Features)
			{
				bool flag = true;
				foreach (ShaderFeatureFlavor flavor in feature.Flavors)
				{
					if (flavor.ID != ShaderFeatureFlavor.EmptyID && !flag)
					{
						list.Add(flavor.ID);
						if (!dictionary.TryGetValue(flavor.ID, out var value))
						{
							value = new List<ShaderGroup>();
							dictionary.Add(flavor.ID, value);
						}
						value.Add(shaderGroup);
					}
					flag = false;
				}
			}
		}
		List<KeyValuePair<string, List<string>>> list2 = new List<KeyValuePair<string, List<string>>>();
		foreach (KeyValuePair<string, List<ShaderGroup>> item in dictionary.OrderByDescending((KeyValuePair<string, List<ShaderGroup>> pair) => pair.Value.Count))
		{
			list2.Add(new KeyValuePair<string, List<string>>(item.Key, new List<string>()));
			list2.Last().Value.Add(item.Key);
		}
		for (int num = 0; num < list2.Count; num++)
		{
			for (int num2 = list2.Count - 1; num2 > 0; num2--)
			{
				string key = list2[num].Key;
				string key2 = list2[num2].Key;
				if (dictionary[key].Intersect(dictionary[key2]).Count() == 0)
				{
					dictionary[key].AddRange(dictionary[key2]);
					dictionary.Remove(key2);
					list2[num].Value.AddRange(list2[num2].Value);
					list2.RemoveAt(num2);
					break;
				}
			}
		}
		uint num3 = 1u;
		foreach (KeyValuePair<string, List<string>> item2 in list2)
		{
			foreach (string item3 in item2.Value)
			{
				FeatureFlavorFlags.Add(item3, num3);
			}
			num3 <<= 1;
		}
	}

	public string CopyrightNotice(string filename)
	{
		return "/**************************************************************************\r\n\r\n    PublicHeader:   Render\r\n    Filename    :   " + Path.GetFileName(filename) + "\r\n    Content     :   " + PlatformName + " Shader descriptors\r\n    Created     :   " + DateTime.Today.Date.ToShortDateString() + "\r\n    Authors     :   Automatically generated.\r\n\r\n    Copyright   :   Copyright " + DateTime.Today.Year + " Autodesk, Inc. All Rights reserved.\r\n                     Copyright " + DateTime.Today.Year + " Final Game Production Inc. All Rights Reserved.\r\n\r\n    Use of this software is subject to the terms of the Autodesk license\r\n    agreement provided at the time of installation or download, or which\r\n    otherwise accompanies this software in either electronic or hard copy form.\r\n\r\n**************************************************************************/\r\n\r\n";
	}

	public virtual void WriteHeaderFile()
	{
		if (!Directory.Exists(Path.GetDirectoryName(PlatformHeaderFilename)))
		{
			Directory.CreateDirectory(Path.GetDirectoryName(PlatformHeaderFilename));
		}
		StreamWriter streamWriter = File.CreateText(PlatformHeaderFilename);
		IndentStreamWriter indentStreamWriter = new IndentStreamWriter(streamWriter);
		indentStreamWriter.Write(CopyrightNotice(PlatformHeaderFilename) + "\n");
		writeHeaderPreamble(indentStreamWriter);
		writeHeaderUniforms(indentStreamWriter);
		writeHeaderFlags(indentStreamWriter);
		writeHeaderStages(indentStreamWriter);
		writeHeaderUniformStructs(indentStreamWriter);
		writeHeaderShaderDescs(indentStreamWriter);
		foreach (ShaderPipeline usedPipeline in UsedPipelineList)
		{
			writeHeaderPipelineShaderDesc(indentStreamWriter, usedPipeline);
		}
		writeHeaderPostamble(indentStreamWriter);
		streamWriter.Close();
		if (CommandLineParser.GetOption<int>(CommandLineParser.Options.Verbosity) != 0)
		{
			Console.WriteLine("Wrote: " + PlatformHeaderFilename);
		}
	}

	public virtual void WriteSourceFile()
	{
		if (!Directory.Exists(Path.GetDirectoryName(PlatformSourceFilename)))
		{
			Directory.CreateDirectory(Path.GetDirectoryName(PlatformSourceFilename));
		}
		StreamWriter streamWriter = File.CreateText(PlatformSourceFilename);
		IndentStreamWriter indentStreamWriter = new IndentStreamWriter(streamWriter);
		indentStreamWriter.Write(CopyrightNotice(PlatformSourceFilename));
		writeSourcePreamble(indentStreamWriter);
		writeSourceUniforms(indentStreamWriter);
		writeSourceShaderDescs(indentStreamWriter);
		uint shadowOffsetStart = 0u;
		Dictionary<int, string> uniformDefDictionary = new Dictionary<int, string>();
		Dictionary<int, string> batchUniformDefDictionary = new Dictionary<int, string>();
		foreach (ShaderPipeline usedPipeline in UsedPipelineList)
		{
			shadowOffsetStart = writeSourcePipelineShaderDescs(usedPipeline, indentStreamWriter, uniformDefDictionary, shadowOffsetStart, batchUniformDefDictionary);
		}
		writeSourcePostamble(indentStreamWriter);
		streamWriter.Close();
		if (CommandLineParser.GetOption<int>(CommandLineParser.Options.Verbosity) != 0)
		{
			Console.WriteLine("Wrote: " + PlatformSourceFilename);
		}
	}

	protected virtual void writeHeaderPreamble(IndentStreamWriter headerFile)
	{
		headerFile.Write("#ifndef INC_SF_Render_" + PlatformName + "_Shaders_H\n");
		headerFile.Write("#define INC_SF_Render_" + PlatformName + "_Shaders_H\n\n");
		headerFile.Write("#include \"Kernel/SF_Types.h\"\n");
		headerFile.Write("#include \"Kernel/SF_Debug.h\"\n");
		headerFile.Write("#if !defined(SF_BUILD_GFXSHADERMAKER)\n");
		headerFile.Write("// This define specifies we are using GFxShaderMaker.\n#define SF_BUILD_GFXSHADERMAKER\n");
		headerFile.Write("#endif\n\n");
		headerFile.Write("#if !defined(SF_RENDER_MAX_BATCHES)\n");
		headerFile.Write("#define SF_RENDER_MAX_BATCHES 24\n");
		headerFile.Write("#endif\n\n");
		headerFile.Write("namespace Scaleform { namespace Render { namespace " + PlatformBase + " {\n\n");
		headerFile.CurrentIndent = "";
		headerFile.Write("const static SInt64 SF_GFXSHADERMAKER_TIMESTAMP = " + DateTime.Now.ToBinary().ToString("D") + "LL;\n\n");
	}

	protected virtual void writeHeaderUniforms(IndentStreamWriter headerFile)
	{
		headerFile.Write("struct Uniform\n");
		headerFile.Write("{\n");
		headerFile.Write("enum Flags\n");
		headerFile.Write("{\n");
		headerFile.Write("Uniform_Builtin  = 1,\n");
		headerFile.Write("Uniform_TexScale = 2,\n");
		headerFile.Write("};\n\n");
		headerFile.Write("enum UniformType\n");
		headerFile.Write("{\n");
		foreach (ShaderVariable uniqueUniform in UniqueUniformList)
		{
			headerFile.Write("SU_" + uniqueUniform.ID + ",\n");
		}
		headerFile.Write("\nSU_Count,\n\n");
		ShaderPipeline.PipelineType pipelineType;
		foreach (ShaderPipeline.PipelineType value in Enum.GetValues(typeof(ShaderPipeline.PipelineType)))
		{
			pipelineType = value;
			uint val = 0u;
			foreach (ShaderVersion requestedShaderVersion in RequestedShaderVersions)
			{
				ShaderPipeline pipeline = requestedShaderVersion.Pipelines.FirstOrDefault((ShaderPipeline p) => p.Type == pipelineType);
				if (pipeline != null)
				{
					val = Math.Max(val, requestedShaderVersion.LinkedSources.FindAll((ShaderLinkedSource src) => src.Pipeline.Type == pipeline.Type).DefaultIfEmpty().Max((ShaderLinkedSource src) => (src != null) ? (src.UniformSize * 4) : 0u));
				}
			}
			string pipelineName = ShaderPipeline.GetPipelineName(pipelineType);
			headerFile.Write("SU_" + pipelineName + "Size = " + val + ",\n");
		}
		headerFile.Write("\n");
		headerFile.Write("SU_TotalSize = ");
		bool flag = true;
		foreach (ShaderPipeline.PipelineType value2 in Enum.GetValues(typeof(ShaderPipeline.PipelineType)))
		{
			if (!flag)
			{
				headerFile.Write(" + ");
			}
			flag = false;
			string pipelineName2 = ShaderPipeline.GetPipelineName(value2);
			headerFile.Write("SU_" + pipelineName2 + "Size");
		}
		headerFile.Write(",\n");
		headerFile.Write("};\n");
		headerFile.Write("static const unsigned char UniformFlags[]; \n");
		headerFile.Write("};\n\n");
		headerFile.Write("extern const char* ShaderUniformNames[Uniform::SU_Count];\n");
	}

	protected virtual void writeHeaderFlags(IndentStreamWriter headerFile)
	{
		headerFile.Write("enum ShaderFlags\n");
		headerFile.Write("{\n");
		List<string> list = new List<string>();
		foreach (ShaderGroup shaderGroup in ShaderGroups)
		{
			foreach (ShaderPermutation permutation in shaderGroup.Permutations)
			{
				foreach (ShaderFeatureFlavor specificFeature in permutation.SpecificFeatures)
				{
					list.AddRange(specificFeature.Flags);
					list.AddRange(specificFeature.PostLink);
				}
			}
		}
		list.Add("User");
		uint num = 1u;
		foreach (string item in list.Distinct())
		{
			headerFile.Write(("Shader_" + item + " = ").PadRight(30) + "0x" + num.ToString("X8") + ",\n");
			num <<= 1;
		}
		headerFile.Write("};\n\n");
	}

	protected virtual void writeHeaderStages(IndentStreamWriter headerFile)
	{
		headerFile.Write("enum ShaderStages\n");
		headerFile.Write("{\n");
		foreach (ShaderPipeline.PipelineType value in Enum.GetValues(typeof(ShaderPipeline.PipelineType)))
		{
			string pipelineName = ShaderPipeline.GetPipelineName(value);
			headerFile.Write("    ShaderStage_" + pipelineName + ",\n");
		}
		headerFile.Write("     ShaderStage_Count,\n");
		headerFile.Write("};\n");
	}

	protected virtual void writeHeaderUniformStructs(IndentStreamWriter headerFile)
	{
		headerFile.Write("struct UniformVar\n");
		headerFile.Write("{\n");
		headerFile.Write("short         Location;\n");
		headerFile.Write("short         ShadowOffset;\n");
		headerFile.Write("unsigned char ElementSize;\n");
		headerFile.Write("short         Size;\n");
		headerFile.Write("unsigned char IsSampler;\n");
		headerFile.Write("unsigned char BatchSize;\n");
		headerFile.Write("};\n\n");
		headerFile.Write("struct BatchVar\n");
		headerFile.Write("{\n");
		headerFile.Write("signed char   Array;\n");
		headerFile.Write("signed char   Offset;\n");
		headerFile.Write("unsigned char Size;\n");
		headerFile.Write("};\n\n");
	}

	protected virtual void writeHeaderShaderDescs(IndentStreamWriter headerFile)
	{
		uint num = 1u;
		uint num2 = 0u;
		headerFile.Write("struct ShaderDesc\n");
		headerFile.Write("{\n");
		headerFile.Write("enum ShaderVersion\n");
		headerFile.Write("{\n");
		foreach (ShaderVersion possibleShaderVersion in PossibleShaderVersions)
		{
			headerFile.Write("ShaderVersion_" + possibleShaderVersion.ID + ",\n");
		}
		headerFile.Write("ShaderVersion_Count,\n");
		headerFile.Write("ShaderVersion_Default = ShaderVersion_" + DefaultShaderVersion.ID + "\n");
		headerFile.Write("};\n\n");
		uint num3 = 0u;
		foreach (ShaderGroup shaderGroup in ShaderGroups)
		{
			num3 = Math.Max(num3, shaderGroup.FeatureFlavorFlags.Values.Max());
		}
		num3 <<= 1;
		headerFile.Write("enum ShaderType\n");
		headerFile.Write("{\n");
		headerFile.Write("ST_None = 0,\n\n");
		num = 1u;
		num2 = 1u;
		List<string> list = new List<string>();
		foreach (ShaderGroup shaderGroup2 in ShaderGroups)
		{
			foreach (string key in shaderGroup2.FeatureFlavorFlags.Keys)
			{
				headerFile.Write(("ST_" + shaderGroup2.ID + "_" + key).PadRight(40) + " = 0x" + shaderGroup2.FeatureFlavorFlags[key].ToString("X8") + ",\n");
			}
			headerFile.Write("\n");
			headerFile.Write("ST_start_" + shaderGroup2.ID + " = " + num + ",\n");
			shaderGroup2.Permutations.Sort();
			foreach (ShaderPermutation permutation in shaderGroup2.Permutations)
			{
				headerFile.Write("ST_" + permutation.GetPermutationName());
				list.Add(permutation.GetPermutationName());
				uint num4 = permutation.ShaderType + num;
				if (num2 + 1 != num4)
				{
					headerFile.Write(" = " + num4);
				}
				num2 = num4;
				headerFile.Write(",\n");
			}
			headerFile.Write("ST_end_" + shaderGroup2.ID + " = " + num2 + ",\n\n");
			num = (num2 + num3) & ~(num3 - 1);
			num2 = 0u;
		}
		headerFile.Write("};\n\n");
		headerFile.Write("enum ShaderIndex\n");
		headerFile.Write("{\n");
		headerFile.Write("STI_None = 0,\n\n");
		foreach (string item in list)
		{
			headerFile.Write("STI_" + item + ",\n");
		}
		headerFile.Write("STI_Count,\n");
		headerFile.Write("};\n");
		writeHeaderShaderDescFunctions(headerFile);
		headerFile.Write("};\n\n");
	}

	protected virtual void writeHeaderShaderDescFunctions(IndentStreamWriter headerFile)
	{
		headerFile.Write("static bool        IsShaderVersionSupported(ShaderVersion ver);\n");
		headerFile.Write("static ShaderType  GetShaderTypeForComboIndex(unsigned comboIndex, ShaderVersion ver = ShaderVersion_Default);\n");
		headerFile.Write("static ShaderIndex GetShaderIndex(ShaderType type);\n");
		headerFile.Write("static UPInt       GetShaderUniformSize(ShaderStages stage);\n");
	}

	protected virtual void writeHeaderPipelineShaderDesc(IndentStreamWriter headerFile, ShaderPipeline pipeline)
	{
		string name = pipeline.Name;
		char letter = pipeline.Letter;
		headerFile.Write("struct " + name + "ShaderDesc\n");
		headerFile.Write("{\n");
		writeHeaderPipelineDescShaderIndex(headerFile, letter, pipeline);
		writeHeaderPipelineDescDataMembers(headerFile, pipeline);
		writeHeaderPipelineDescFunctions(headerFile, pipeline);
		headerFile.Write("};\n\n");
	}

	protected abstract void writeHeaderPipelineShaderDataMembers(IndentStreamWriter headerFile, ShaderPipeline pipeline);

	protected virtual void writeHeaderPipelineDescDataMembers(IndentStreamWriter headerFile, ShaderPipeline pipeline)
	{
		headerFile.Write("ShaderDesc::ShaderType      Type;\n");
		headerFile.Write("ShaderDesc::ShaderVersion   Version;\n");
		headerFile.Write("unsigned                    Flags;\n");
		headerFile.Write("const UniformVar*           Uniforms;\n");
		headerFile.Write("const BatchVar*             BatchUniforms;\n\n");
		writeHeaderPipelineShaderDataMembers(headerFile, pipeline);
		string[] array = GeneratePipelineHeaderExtras(pipeline).Split("\n".ToCharArray());
		foreach (string text in array)
		{
			headerFile.Write(text + "\n");
		}
		string name = pipeline.Name;
		char letter = pipeline.Letter;
		headerFile.Write("static const " + name + "ShaderDesc* Descs[" + letter + "SI_Count];\n");
	}

	protected virtual void writeHeaderPipelineDescFunctions(IndentStreamWriter headerFile, ShaderPipeline pipeline)
	{
		string name = pipeline.Name;
		_ = pipeline.Letter;
		headerFile.Write("static const " + name + "ShaderDesc* GetDesc(ShaderDesc::ShaderType shader, ShaderDesc::ShaderVersion ver = ShaderDesc::ShaderVersion_Default);\n");
		headerFile.Write("static ShaderIndex GetShaderIndex(ShaderDesc::ShaderType shader, ShaderDesc::ShaderVersion ver = ShaderDesc::ShaderVersion_Default);\n");
		headerFile.Write("static ShaderIndex GetShaderIndex(ShaderDesc::ShaderIndex shader, ShaderDesc::ShaderVersion ver = ShaderDesc::ShaderVersion_Default);\n");
		headerFile.Write("static unsigned    GetShaderComboIndex(ShaderDesc::ShaderType shader, ShaderDesc::ShaderVersion ver = ShaderDesc::ShaderVersion_Default);\n");
		headerFile.Write("static ShaderIndex GetShaderIndexForComboIndex(unsigned index, ShaderDesc::ShaderVersion ver = ShaderDesc::ShaderVersion_Default);\n");
	}

	protected virtual void writeHeaderPipelineDescShaderIndex(IndentStreamWriter headerFile, char shaderLetter, ShaderPipeline pipeline)
	{
		headerFile.Write("enum ShaderIndex\n");
		headerFile.Write("{\n");
		headerFile.Write(shaderLetter + "SI_None = 0,\n\n");
		int num = 1;
		foreach (ShaderVersion requestedShaderVersion in RequestedShaderVersions)
		{
			foreach (List<ShaderLinkedSource> value in requestedShaderVersion.LinkedSourceUniqueDescs.Values)
			{
				bool flag = false;
				foreach (ShaderLinkedSource item in value.FindAll((ShaderLinkedSource s) => s.Pipeline.Type == pipeline.Type))
				{
					headerFile.Write((shaderLetter + "SI_" + requestedShaderVersion.ID + "_" + item.ID).PadRight(50) + " = " + num + ",\n");
					flag = true;
				}
				if (flag)
				{
					num++;
				}
			}
		}
		headerFile.Write(shaderLetter + "SI_Count\n");
		headerFile.Write("};\n\n");
	}

	protected virtual void writeHeaderPostamble(IndentStreamWriter headerFile)
	{
		headerFile.Write("static const unsigned UniqueShaderCombinations = " + RequestedShaderVersions.Max((ShaderVersion ver) => ver.LinkedSources.Max((ShaderLinkedSource s) => s.ShaderComboIndex) + 1) + ";\n\n");
		headerFile.Write("}}} // Scaleform::Render::" + PlatformBase + "\n\n");
		headerFile.Write("#endif\n");
	}

	protected virtual void writeSourcePreamble(IndentStreamWriter sourceFile)
	{
		sourceFile.Write("#include \"Render/" + PlatformBase + "/" + Path.GetFileName(PlatformHeaderFilename) + "\"\n");
		sourceFile.Write("#include \"Render/" + PlatformBase + "/" + PlatformBase + "_Shader.h\"\n");
		sourceFile.Write("#include \"Render/Render_Vertex.h\"\n");
		sourceFile.Write("namespace Scaleform { namespace Render { namespace " + PlatformBase + " {\n\n");
		sourceFile.CurrentIndent = "";
	}

	protected virtual void writeSourceUniforms(IndentStreamWriter sourceFile)
	{
		sourceFile.Write("const char* ShaderUniformNames[Uniform::SU_Count] = \n{\n");
		List<ShaderVariable> uniqueUniformList = UniqueUniformList;
		foreach (ShaderVariable item in uniqueUniformList)
		{
			sourceFile.Write("\"" + item.ID + "\",\n");
		}
		sourceFile.Write("};\n\n");
		sourceFile.Write("const unsigned char Uniform::UniformFlags[Uniform::SU_Count] = \n{\n");
		foreach (ShaderVariable item2 in uniqueUniformList)
		{
			sourceFile.Write("0,  // " + item2.ID + "\n");
		}
		sourceFile.Write("};\n\n");
	}

	protected virtual void writeSourceShaderDescs(IndentStreamWriter sourceFile)
	{
		writeSourceShaderDescFunctions(sourceFile);
		sourceFile.Write("static const int ShaderVersionIndex[ShaderDesc::ShaderVersion_Count] =\n{\n");
		uint num = 0u;
		ShaderVersion ver;
		foreach (ShaderVersion possibleShaderVersion in PossibleShaderVersions)
		{
			ver = possibleShaderVersion;
			if (RequestedShaderVersions.Count((ShaderVersion v) => v.ID == ver.ID) > 0)
			{
				sourceFile.Write(num++ + ",\n");
			}
			else
			{
				sourceFile.Write("-1,\n");
			}
		}
		sourceFile.Write("};\n\n");
		sourceFile.Write("static const unsigned ShaderVersionsSupported = " + num + ";\n\n");
		sourceFile.Write("struct ShaderIndexEntry\n");
		sourceFile.Write("{\n");
		foreach (ShaderPipeline usedPipeline in UsedPipelineList)
		{
			sourceFile.Write(usedPipeline.Name + "ShaderDesc::ShaderIndex " + usedPipeline.Name + "Index;\n");
		}
		sourceFile.Write("};\n\n");
		sourceFile.Write("static const ShaderIndexEntry ShaderIndexingData[ShaderDesc::STI_Count][ShaderVersionsSupported] = \n{\n");
		sourceFile.Write("{ // ST_None\n");
		ShaderVersion ver2;
		foreach (ShaderVersion possibleShaderVersion2 in PossibleShaderVersions)
		{
			ver2 = possibleShaderVersion2;
			if (RequestedShaderVersions.Count((ShaderVersion v) => v.ID == ver2.ID) == 0)
			{
				continue;
			}
			sourceFile.Write("{ // ShaderVersion_" + ver2.ID + "\n");
			foreach (ShaderPipeline usedPipeline2 in UsedPipelineList)
			{
				sourceFile.Write(usedPipeline2.Name + "ShaderDesc::" + usedPipeline2.Letter + "SI_None");
				sourceFile.Write((usedPipeline2 != UsedPipelineList.Last()) ? ",\n" : "\n");
			}
			sourceFile.Write("},\n");
		}
		sourceFile.Write("},\n");
		foreach (ShaderGroup shaderGroup in ShaderGroups)
		{
			foreach (ShaderPermutation permutation in shaderGroup.Permutations)
			{
				sourceFile.Write("{ // ST_" + permutation.GetPermutationName() + "\n");
				ShaderVersion ver3;
				foreach (ShaderVersion possibleShaderVersion3 in PossibleShaderVersions)
				{
					ver3 = possibleShaderVersion3;
					if (RequestedShaderVersions.Count((ShaderVersion v) => v.ID == ver3.ID) == 0)
					{
						continue;
					}
					sourceFile.Write("{ // ShaderVersion_" + ver3.ID + "\n");
					ShaderPipeline pipeline;
					foreach (ShaderPipeline usedPipeline3 in UsedPipelineList)
					{
						pipeline = usedPipeline3;
						string text = ((!permutation.IsValid(ver3) || permutation.LinkedSources.Find((ShaderLinkedSource src) => src.Pipeline.Type == pipeline.Type) == null) ? (pipeline.Name + "ShaderDesc::" + pipeline.Letter + "SI_None") : (pipeline.Name + "ShaderDesc::" + pipeline.Letter + "SI_" + ver3.ID + "_" + pipeline.Letter + permutation.GetPermutationName()));
						sourceFile.Write(text + ((pipeline != UsedPipelineList.Last()) ? ",\n" : "\n"));
					}
					sourceFile.Write((ver3 == RequestedShaderVersions.Last()) ? "}\n" : "},\n");
				}
				sourceFile.Write("},\n");
			}
		}
		sourceFile.Write("};\n");
	}

	protected virtual void writeSourceShaderDescFunctions(IndentStreamWriter sourceFile)
	{
		sourceFile.Write("bool ShaderDesc::IsShaderVersionSupported(ShaderVersion ver)\n");
		sourceFile.Write("{\n");
		sourceFile.Write("switch(ver)\n");
		sourceFile.Write("{\n");
		foreach (ShaderVersion requestedShaderVersion in RequestedShaderVersions)
		{
			sourceFile.Write("case ShaderVersion_" + requestedShaderVersion.ID + ": return true;\n");
		}
		sourceFile.Write("default: return false;\n");
		sourceFile.Write("}\n");
		sourceFile.Write("};\n\n");
		sourceFile.Write("ShaderDesc::ShaderType ShaderDesc::GetShaderTypeForComboIndex(unsigned comboIndex, ShaderVersion ver)\n");
		sourceFile.Write("{\n");
		sourceFile.Write("switch(ver)\n");
		sourceFile.Write("{\n");
		foreach (ShaderVersion requestedShaderVersion2 in RequestedShaderVersions)
		{
			sourceFile.Write("case ShaderDesc::ShaderVersion_" + requestedShaderVersion2.ID + ":\n");
			if (!requestedShaderVersion2.RequireShaderCombos)
			{
				sourceFile.Write("{\n");
				sourceFile.Write("SF_UNUSED(comboIndex);\n");
				sourceFile.Write("SF_DEBUG_ASSERT(0, \"" + requestedShaderVersion2.ID + " indicated that ShaderCombo information was not required.\");\n");
				sourceFile.Write("return ST_None;\n");
				sourceFile.Write("}\n");
				continue;
			}
			sourceFile.Write("switch(comboIndex)\n");
			sourceFile.Write("{\n");
			Dictionary<uint, string> dictionary = new Dictionary<uint, string>();
			foreach (ShaderLinkedSource linkedSource in requestedShaderVersion2.LinkedSources)
			{
				if (!dictionary.TryGetValue(linkedSource.ShaderComboIndex, out var _))
				{
					dictionary.Add(linkedSource.ShaderComboIndex, linkedSource.ID.Remove(0, 1));
				}
			}
			List<uint> list = dictionary.Keys.ToList();
			list.Sort();
			foreach (uint item in list)
			{
				sourceFile.Write("case " + item + ": return ST_" + dictionary[item] + ";\n");
			}
			sourceFile.Write("default: return ST_None;\n");
			sourceFile.Write("}\n");
		}
		sourceFile.Write("default: SF_DEBUG_ASSERT1(0, \"Invalid shader platform provided (%d)\", ver); return ST_None;\n");
		sourceFile.Write("}\n};\n\n");
		sourceFile.Write("ShaderDesc::ShaderIndex ShaderDesc::GetShaderIndex(ShaderType type)\n{\n");
		sourceFile.Write("switch(type)\n{\n");
		foreach (ShaderGroup shaderGroup in ShaderGroups)
		{
			foreach (ShaderPermutation permutation in shaderGroup.Permutations)
			{
				sourceFile.Write("case ST_" + permutation.GetPermutationName() + ": " + ("return STI_" + permutation.GetPermutationName() + ";\n").PadLeft(60));
			}
		}
		sourceFile.Write("default: SF_DEBUG_ASSERT1(0, \"Invalid ShaderType (%d)\", type);\n");
		sourceFile.Write("}\n");
		sourceFile.Write("return STI_None;\n");
		sourceFile.Write("}\n\n");
		sourceFile.Write("UPInt ShaderDesc::GetShaderUniformSize(ShaderStages stage)\n");
		sourceFile.Write("{\n");
		sourceFile.Write("switch(stage)\n{\n");
		foreach (ShaderPipeline.PipelineType value2 in Enum.GetValues(typeof(ShaderPipeline.PipelineType)))
		{
			string pipelineName = ShaderPipeline.GetPipelineName(value2);
			sourceFile.Write("case ShaderStage_" + pipelineName + ": return Uniform::SU_" + pipelineName + "Size;\n");
		}
		sourceFile.Write("default: SF_DEBUG_ASSERT1(0, \"Unexpected shader stage (%d)\\n\", stage); return 0;\n");
		sourceFile.Write("}\n");
		sourceFile.Write("}\n\n");
	}

	protected virtual uint writeSourcePipelineShaderDescs(ShaderPipeline pipeline, IndentStreamWriter sourceFile, Dictionary<int, string> uniformDefDictionary, uint shadowOffsetStart, Dictionary<int, string> batchUniformDefDictionary)
	{
		string name = pipeline.Name;
		char c = name[0];
		foreach (ShaderVersion requestedShaderVersion in RequestedShaderVersions)
		{
			List<ShaderLinkedSource> list = new List<ShaderLinkedSource>();
			foreach (List<ShaderLinkedSource> value in requestedShaderVersion.LinkedSourceUniqueDescs.Values)
			{
				if (value.Count > 0 && value.First().Pipeline.Type == pipeline.Type)
				{
					list.Add(value.First());
					writeSourcePipelineShaderGlobals(sourceFile, requestedShaderVersion, pipeline, value.First().SourceCodeDuplicateID);
				}
			}
			sourceFile.Write("\n");
			List<ShaderVariable> uniqueUniformList = UniqueUniformList;
			foreach (ShaderLinkedSource item in list)
			{
				string uniformDefs = "";
				ulong num = GenerateUniformDefs(ref uniformDefDictionary, ref uniqueUniformList, item, shadowOffsetStart, ref uniformDefs);
				if (uniformDefs != null)
				{
					sourceFile.Write("UniformVar Uniforms_" + num + "[Uniform::SU_Count] = \n{\n");
					string[] array = uniformDefs.Split("\n".ToCharArray(), StringSplitOptions.RemoveEmptyEntries);
					foreach (string text in array)
					{
						sourceFile.Write(text + "\n");
					}
					sourceFile.Write("};\n\n");
				}
				num = GenerateBatchUniformDefs(ref batchUniformDefDictionary, ref uniqueUniformList, item, ref uniformDefs);
				if (uniformDefs != null)
				{
					sourceFile.Write("BatchVar BatchUniforms_" + num + "[Uniform::SU_Count] = \n{\n");
					string[] array2 = uniformDefs.Split("\n".ToCharArray(), StringSplitOptions.RemoveEmptyEntries);
					foreach (string text2 in array2)
					{
						sourceFile.Write(text2 + "\n");
					}
					sourceFile.Write("};\n\n");
				}
			}
			foreach (ShaderLinkedSource item2 in list)
			{
				writeSourcePipelineDescDataMembers(sourceFile, requestedShaderVersion, item2, pipeline, uniformDefDictionary, uniqueUniformList, shadowOffsetStart, batchUniformDefDictionary);
			}
		}
		sourceFile.Write("const " + name + "ShaderDesc* " + name + "ShaderDesc::Descs[" + c + "SI_Count] = {\n");
		sourceFile.Write("0,\n");
		int num2 = 1;
		foreach (ShaderVersion requestedShaderVersion2 in RequestedShaderVersions)
		{
			foreach (List<ShaderLinkedSource> value2 in requestedShaderVersion2.LinkedSourceUniqueDescs.Values)
			{
				ShaderLinkedSource shaderLinkedSource = value2.FindAll((ShaderLinkedSource s) => s.Pipeline.Type == pipeline.Type).FirstOrDefault();
				if (shaderLinkedSource != null)
				{
					sourceFile.Write(("&ShaderDesc_" + c + "S_" + requestedShaderVersion2.ID + "_" + shaderLinkedSource.ID + ",").PadRight(60) + " // " + num2 + "\n");
					num2++;
				}
			}
		}
		sourceFile.Write("};\n\n");
		writeSourcePipelineDescFunctions(sourceFile, pipeline);
		shadowOffsetStart += RequestedShaderVersions.Max((ShaderVersion ver) => ver.LinkedSources.FindAll((ShaderLinkedSource src) => src.Pipeline.Type == pipeline.Type).DefaultIfEmpty().Max((ShaderLinkedSource src) => (src != null) ? (src.UniformSize * 4) : 0u));
		return shadowOffsetStart;
	}

	protected abstract void writeSourcePipelineShaderGlobals(IndentStreamWriter sourceFile, ShaderVersion ver, ShaderPipeline pipeline, string srcID);

	protected virtual void writeSourcePipelineDescDataMembers(IndentStreamWriter sourceFile, ShaderVersion ver, ShaderLinkedSource src, ShaderPipeline pipeline, Dictionary<int, string> uniformDefDictionary, List<ShaderVariable> uniqueUniformList, uint shadowOffsetStart, Dictionary<int, string> batchUniformDefDictionary)
	{
		string name = pipeline.Name;
		char c = name[0];
		sourceFile.Write("static " + name + "ShaderDesc ShaderDesc_" + c + "S_" + ver.ID + "_" + src.ID + " = \n{\n");
		sourceFile.Write("/* Type */          ShaderDesc::ST_" + src.ID.Remove(0, 1) + ",\n");
		sourceFile.Write("/* Version */       ShaderDesc::ShaderVersion_" + ver.ID + ",\n");
		sourceFile.Write("/* Flags */         ");
		if (src.Flags.Count == 0)
		{
			sourceFile.Write("0,\n");
		}
		else
		{
			bool flag = true;
			foreach (string flag2 in src.Flags)
			{
				if (!flag)
				{
					sourceFile.Write(" | ");
				}
				sourceFile.Write("Shader_" + flag2);
				flag = false;
			}
			sourceFile.Write(",\n");
		}
		string uniformDefs = "";
		sourceFile.Write("/* Uniforms */      Uniforms_" + GenerateUniformDefs(ref uniformDefDictionary, ref uniqueUniformList, src, shadowOffsetStart, ref uniformDefs) + ",\n");
		sourceFile.Write("/* BatchUniforms */ BatchUniforms_" + GenerateBatchUniformDefs(ref batchUniformDefDictionary, ref uniqueUniformList, src, ref uniformDefs) + ",\n");
		writeSourcePipelineShaderData(sourceFile, ver, pipeline, src);
		string[] array = GeneratePipelineSourceExtras(ver, pipeline, src).Split("\n".ToCharArray());
		foreach (string text in array)
		{
			sourceFile.Write(text + "\n");
		}
		sourceFile.Write("};\n\n");
	}

	protected abstract void writeSourcePipelineShaderData(IndentStreamWriter sourceFile, ShaderVersion ver, ShaderPipeline pipeline, ShaderLinkedSource src);

	protected virtual void writeSourcePipelineDescFunctions(IndentStreamWriter sourceFile, ShaderPipeline pipeline)
	{
		string name = pipeline.Name;
		char c = name[0];
		sourceFile.Write("const " + name + "ShaderDesc* " + name + "ShaderDesc::GetDesc(ShaderDesc::ShaderType shader, ShaderDesc::ShaderVersion ver)\n");
		sourceFile.Write("{\n");
		sourceFile.Write("return Descs[GetShaderIndex(shader, ver)];\n");
		sourceFile.Write("};\n\n");
		sourceFile.Write(name + "ShaderDesc::ShaderIndex " + name + "ShaderDesc::GetShaderIndex(ShaderDesc::ShaderType shader, ShaderDesc::ShaderVersion ver)\n{\n");
		sourceFile.Write("ShaderDesc::ShaderIndex index = ShaderDesc::GetShaderIndex(shader);\n");
		sourceFile.Write("int shaderVersionIndex = ShaderVersionIndex[ver];\n");
		sourceFile.Write("SF_DEBUG_ASSERT1(shaderVersionIndex >= 0, \"ShaderVersion did not have support compiled in (%d)\", ver);\n");
		sourceFile.Write("if (shaderVersionIndex < 0) shaderVersionIndex = 0;\n");
		sourceFile.Write("return ShaderIndexingData[index][shaderVersionIndex]. " + name + "Index;\n");
		sourceFile.Write("};\n\n");
		sourceFile.Write(name + "ShaderDesc::ShaderIndex " + name + "ShaderDesc::GetShaderIndex(ShaderDesc::ShaderIndex shader, ShaderDesc::ShaderVersion ver)\n{\n");
		sourceFile.Write("int shaderVersionIndex = ShaderVersionIndex[ver];\n");
		sourceFile.Write("SF_DEBUG_ASSERT1(shaderVersionIndex >= 0, \"ShaderVersion did not have support compiled in (%d)\", ver);\n");
		sourceFile.Write("if (shaderVersionIndex < 0) shaderVersionIndex = 0;\n");
		sourceFile.Write("return ShaderIndexingData[shader][shaderVersionIndex]. " + name + "Index;\n");
		sourceFile.Write("};\n\n");
		sourceFile.Write(name + "ShaderDesc::ShaderIndex " + name + "ShaderDesc::GetShaderIndexForComboIndex(unsigned index, ShaderDesc::ShaderVersion ver) \n");
		sourceFile.Write("{\n");
		sourceFile.Write("switch(ver)\n");
		sourceFile.Write("{\n");
		foreach (ShaderVersion requestedShaderVersion in RequestedShaderVersions)
		{
			sourceFile.Write("case ShaderDesc::ShaderVersion_" + requestedShaderVersion.ID + ":\n");
			if (!requestedShaderVersion.RequireShaderCombos)
			{
				sourceFile.Write("{\n");
				sourceFile.Write("SF_UNUSED(index);\n");
				sourceFile.Write("SF_DEBUG_ASSERT(0, \"" + requestedShaderVersion.ID + " indicated that ShaderCombo information was not required.\");\n");
				sourceFile.Write("return " + c + "SI_None;\n");
				sourceFile.Write("}\n");
				continue;
			}
			sourceFile.Write("switch(index)\n");
			sourceFile.Write("{\n");
			uint num = requestedShaderVersion.LinkedSources.Max((ShaderLinkedSource s) => s.ShaderComboIndex);
			uint comboIndex;
			for (comboIndex = 0u; comboIndex <= num; comboIndex++)
			{
				ShaderLinkedSource shaderLinkedSource = requestedShaderVersion.LinkedSources.Find((ShaderLinkedSource s) => s.ShaderComboIndex == comboIndex && s.Pipeline.Type == pipeline.Type);
				if (shaderLinkedSource != null)
				{
					sourceFile.Write(("case " + comboIndex + ": ").PadRight(20) + "return " + c + "SI_" + requestedShaderVersion.ID + "_" + shaderLinkedSource.ID + ";\n");
				}
			}
			sourceFile.Write("default: SF_ASSERT(0); return " + c + "SI_None;\n");
			sourceFile.Write("}\n");
		}
		sourceFile.Write("default: SF_ASSERT(0); return " + c + "SI_None;\n");
		sourceFile.Write("}\n");
		sourceFile.Write("}\n\n");
		sourceFile.Write("unsigned " + name + "ShaderDesc::GetShaderComboIndex(ShaderDesc::ShaderType shader, ShaderDesc::ShaderVersion ver)\n");
		sourceFile.Write("{\n");
		sourceFile.Write("switch(ver)\n");
		sourceFile.Write("{\n");
		foreach (ShaderVersion requestedShaderVersion2 in RequestedShaderVersions)
		{
			sourceFile.Write("case ShaderDesc::ShaderVersion_" + requestedShaderVersion2.ID + ":\n");
			if (!requestedShaderVersion2.RequireShaderCombos)
			{
				sourceFile.Write("{\n");
				sourceFile.Write("SF_UNUSED(shader);\n");
				sourceFile.Write("SF_DEBUG_ASSERT(0, \"" + requestedShaderVersion2.ID + " indicated that ShaderCombo information was not required.\");\n");
				sourceFile.Write("return 0;\n");
				sourceFile.Write("}\n");
				continue;
			}
			sourceFile.Write("switch(shader)\n");
			sourceFile.Write("{\n");
			foreach (ShaderGroup shaderGroup in ShaderGroups)
			{
				shaderGroup.Permutations.Sort();
				ShaderPermutation permu;
				foreach (ShaderPermutation permutation in shaderGroup.Permutations)
				{
					permu = permutation;
					foreach (ShaderLinkedSource item in requestedShaderVersion2.LinkedSources.FindAll((ShaderLinkedSource s) => s.ID.Remove(0, 1) == permu.GetPermutationName() && s.Pipeline.Type == pipeline.Type))
					{
						sourceFile.Write("case " + ("ShaderDesc::ST_" + permu.GetPermutationName() + ": ").PadRight(60) + "return " + item.ShaderComboIndex + ";\n");
					}
				}
			}
			sourceFile.Write("default: SF_ASSERT(0); return 0;\n");
			sourceFile.Write("}\n");
		}
		sourceFile.Write("default: SF_ASSERT(0); return 0;\n");
		sourceFile.Write("}\n");
		sourceFile.Write("}\n\n");
	}

	protected virtual void writeSourcePostamble(IndentStreamWriter sourceFile)
	{
		sourceFile.Write("}}} // Scaleform::Render::" + PlatformBase + "\n\n");
	}

	protected ulong GenerateUniformDefs(ref Dictionary<int, string> unidict, ref List<ShaderVariable> uniqueUniformList, ShaderLinkedSource src, uint shadowOffsetStart, ref string uniformDefs)
	{
		uniformDefs = "";
		ShaderVariable var;
		foreach (ShaderVariable item in uniqueUniformList.Distinct())
		{
			var = item;
			uniformDefs = uniformDefs + "/* " + var.ID + "*/".PadRight(13 - var.ID.Length);
			ShaderVariable shaderVariable = src.VariableList.Find((ShaderVariable v) => v.ID == var.ID && v.VarType == ShaderVariable.VariableType.Variable_Uniform);
			if (shaderVariable == null)
			{
				uniformDefs += "{ -1, 0, 0, 0, 0, 0 },\n";
				continue;
			}
			List<ShaderVariable> source = src.VariableList.FindAll((ShaderVariable v) => Regex.IsMatch(v.Semantic, "^" + var.ID));
			uint num = (uint)source.Sum((ShaderVariable v) => v.ArraySize);
			string text = uniformDefs;
			uniformDefs = text + "{" + shaderVariable.BaseRegister + ", " + (shadowOffsetStart + shaderVariable.BaseRegister * 4) + ", " + shaderVariable.ElementCount + ", " + shaderVariable.Size + ", " + (shaderVariable.SamplerType ? "1" : "0") + ", " + num + " },\n";
		}
		int hashCode = uniformDefs.GetHashCode();
		if (!unidict.ContainsKey(hashCode))
		{
			unidict.Add(hashCode, uniformDefs);
		}
		else
		{
			uniformDefs = null;
		}
		return Convert.ToUInt64(hashCode + uint.MaxValue);
	}

	protected ulong GenerateBatchUniformDefs(ref Dictionary<int, string> unidict, ref List<ShaderVariable> uniqueUniformList, ShaderLinkedSource src, ref string batchUniformDefs)
	{
		batchUniformDefs = "";
		ShaderVariable var;
		foreach (ShaderVariable item in uniqueUniformList.Distinct())
		{
			var = item;
			batchUniformDefs = batchUniformDefs + "/* " + var.ID + "*/".PadRight(13 - var.ID.Length);
			ShaderVariable shaderVariable = src.VariableList.Find((ShaderVariable v) => v.ID == var.ID && v.VarType == ShaderVariable.VariableType.Variable_VirtualUniform);
			if (shaderVariable == null)
			{
				batchUniformDefs += " {Uniform::SU_Count, -1, 0},\n";
				continue;
			}
			string[] array = shaderVariable.Semantic.Split(":".ToCharArray(), StringSplitOptions.RemoveEmptyEntries);
			object obj = batchUniformDefs;
			batchUniformDefs = string.Concat(obj, " {Uniform::SU_", array[0], ", ", array[1], ", ", shaderVariable.ArraySize, "},\n");
		}
		int hashCode = batchUniformDefs.GetHashCode();
		if (!unidict.ContainsKey(hashCode))
		{
			unidict.Add(hashCode, batchUniformDefs);
		}
		else
		{
			batchUniformDefs = null;
		}
		return Convert.ToUInt64(hashCode + uint.MaxValue);
	}

	public virtual string GeneratePipelineHeaderExtras(ShaderPipeline pipeline)
	{
		string text = "";
		switch (pipeline.Type)
		{
		case ShaderPipeline.PipelineType.Vertex:
		{
			int maximumVertexAttributes = MaximumVertexAttributes;
			text += "struct VertexAttrDesc\n";
			text += "{\n";
			text += "    const char*   Name;\n";
			text += "    unsigned      Attr;\n";
			text += "};\n\n";
			text += "char           NumAttribs;\n";
			text += "enum {\n";
			object obj2 = text;
			text = string.Concat(obj2, "    MaxVertexAttributes = ", maximumVertexAttributes, "\n");
			text += "};\n";
			text += "VertexAttrDesc Attributes[MaxVertexAttributes];\n";
			break;
		}
		case ShaderPipeline.PipelineType.Fragment:
		{
			int num = 0;
			foreach (ShaderVersion requestedShaderVersion in RequestedShaderVersions)
			{
				foreach (List<ShaderLinkedSource> value in requestedShaderVersion.LinkedSourceUniqueDescs.Values)
				{
					num = Math.Max(num, value.Max((ShaderLinkedSource src) => src.VariableList.FindAll((ShaderVariable v) => v.SamplerType).Sum((ShaderVariable var) => (int)((var.ArraySize == 0) ? 1 : var.ArraySize))));
				}
			}
			object obj = text;
			text = string.Concat(obj, "enum { MaxTextureSamplers = ", num, " };\n");
			break;
		}
		}
		return text;
	}

	public virtual string GeneratePipelineSourceExtras(ShaderVersion ver, ShaderPipeline pipeline, ShaderLinkedSource src)
	{
		string text = "";
		string text2 = "";
		if (pipeline.Type == ShaderPipeline.PipelineType.Vertex)
		{
			List<ShaderVariable> sortedAttributeList = src.SortedAttributeList;
			text = text + "/* NumAttribs */    " + sortedAttributeList.Count + ",\n";
			text += "/* Attributes */    {\n";
			text2 += "                      ";
			foreach (ShaderVariable item in sortedAttributeList)
			{
				string text3 = "VET_Color";
				string semantic = item.Semantic;
				text3 = Regex.Replace(semantic, "\\d+$", "") switch
				{
					"COLOR" => "VET_Color", 
					"FACTOR" => "VET_Color | (1 << VET_Index_Shift)", 
					"TEXCOORD" => "VET_TexCoord", 
					"INSTANCE" => "VET_Instance8", 
					_ => "VET_Pos", 
				};
				object obj = text;
				text = string.Concat(obj, "{ \"", item.ID, "\", ".PadRight(13 - item.ID.Length), item.ElementCount, " | ", text3, "},\n");
			}
			text += "},\n";
			text2 = text2.Remove(0, 22);
		}
		return text;
	}

	public virtual void WriteShaderSources()
	{
		if (!Directory.Exists(PlatformObjDirectory))
		{
			Directory.CreateDirectory(PlatformObjDirectory);
		}
		Dictionary<string, string> dictionary = new Dictionary<string, string>();
		foreach (ShaderVersion requestedShaderVersion in RequestedShaderVersions)
		{
			foreach (ShaderLinkedSource value2 in requestedShaderVersion.LinkedSourceDuplicates.Values)
			{
				string text = Path.Combine(PlatformObjDirectory, requestedShaderVersion.GetShaderFilename(value2));
				if (dictionary.TryGetValue(text, out var _))
				{
					throw new Exception("Two shader sources share the same filename (" + text + ")");
				}
				StreamWriter streamWriter = File.CreateText(text);
				streamWriter.Write(requestedShaderVersion.GetSourceCodeContent(value2));
				streamWriter.Close();
			}
		}
	}

	public abstract void CreateShaderOutput();

	protected virtual void CompileSingleShaderImpl(CompileThreadData data)
	{
		throw new Exception("Platform does not support compiling binary shaders.");
	}

	protected int launchProcess(string executable, string args, out string stdout, out string stderr)
	{
		return launchProcess(executable, args, Environment.CurrentDirectory, out stdout, out stderr);
	}

	protected int launchProcess(string executable, string args, string workingDir, out string stdout, out string stderr)
	{
		Process process = new Process();
		process.StartInfo.FileName = executable;
		process.StartInfo.Arguments = args;
		process.StartInfo.ErrorDialog = false;
		process.StartInfo.CreateNoWindow = true;
		process.StartInfo.UseShellExecute = false;
		process.StartInfo.RedirectStandardError = true;
		process.StartInfo.RedirectStandardOutput = true;
		process.StartInfo.WorkingDirectory = workingDir;
		StringBuilder stringBuilder = new StringBuilder();
		StringBuilder stringBuilder2 = new StringBuilder();
		if (CommandLineParser.GetOption<int>(CommandLineParser.Options.Verbosity) >= 2)
		{
			Console.WriteLine(executable + " " + args);
		}
		process.OutputDataReceived += proc_OutputDataReceived;
		process.ErrorDataReceived += proc_ErrorDataReceived;
		process.Start();
		lock (launchProcessStdOuts)
		{
			launchProcessStdOuts.Add(process.Id, stringBuilder);
		}
		lock (launchProcessStdErrs)
		{
			launchProcessStdErrs.Add(process.Id, stringBuilder2);
		}
		process.BeginOutputReadLine();
		process.BeginErrorReadLine();
		process.WaitForExit();
		stdout = stringBuilder.ToString();
		stderr = stringBuilder2.ToString();
		lock (launchProcessStdOuts)
		{
			launchProcessStdOuts.Remove(process.Id);
		}
		lock (launchProcessStdErrs)
		{
			launchProcessStdErrs.Remove(process.Id);
		}
		int exitCode = process.ExitCode;
		process.Close();
		if (CommandLineParser.GetOption<int>(CommandLineParser.Options.Verbosity) >= 2)
		{
			Console.WriteLine(stderr);
		}
		if (CommandLineParser.GetOption<int>(CommandLineParser.Options.Verbosity) >= 3)
		{
			Console.WriteLine(stdout);
		}
		return exitCode;
	}

	private static void proc_OutputDataReceived(object sender, DataReceivedEventArgs e)
	{
		Process process = sender as Process;
		if (e.Data == null)
		{
			return;
		}
		lock (launchProcessStdOuts)
		{
			if (launchProcessStdOuts.TryGetValue(process.Id, out var value))
			{
				value.Append(e.Data + "\n");
			}
		}
	}

	private static void proc_ErrorDataReceived(object sender, DataReceivedEventArgs e)
	{
		Process process = sender as Process;
		if (e.Data == null)
		{
			return;
		}
		lock (launchProcessStdErrs)
		{
			if (launchProcessStdErrs.TryGetValue(process.Id, out var value))
			{
				value.Append(e.Data + "\n");
			}
		}
	}

	protected void CompileSingleShader(object data)
	{
		CompileThreadData compileThreadData = data as CompileThreadData;
		try
		{
			compileThreadData.This.CompileSingleShaderImpl(compileThreadData);
		}
		catch (Exception exception)
		{
			if (compileThreadData != null)
			{
				compileThreadData.Exception = exception;
			}
		}
	}

	protected void CompileShadersThreaded(List<CompileThreadData> threadData)
	{
		List<Thread> list = new List<Thread>();
		foreach (CompileThreadData threadDatum in threadData)
		{
			Thread thread = new Thread(CompileSingleShader);
			thread.Start(threadDatum);
			list.Add(thread);
			foreach (Thread item in list)
			{
				if (!item.IsAlive)
				{
					list.Remove(item);
					break;
				}
			}
			if (list.Count > MaximumSimultaneousCompileThreads)
			{
				list[0].Join();
				list.RemoveAt(0);
			}
		}
		while (list.Count > 0)
		{
			list[0].Join();
			list.RemoveAt(0);
		}
		bool flag = false;
		foreach (CompileThreadData threadDatum2 in threadData)
		{
			if (threadDatum2.Exception != null)
			{
				throw threadDatum2.Exception;
			}
			if (CommandLineParser.GetOption<int>(CommandLineParser.Options.Verbosity) >= 2)
			{
				Console.WriteLine("Compiling Shader: " + threadDatum2.ShaderFilename);
				if (CommandLineParser.GetOption<int>(CommandLineParser.Options.Verbosity) >= 3)
				{
					Console.WriteLine(threadDatum2.CommandLine);
					Console.WriteLine(threadDatum2.StdError);
					Console.WriteLine(threadDatum2.StdOutput);
				}
			}
			if (threadDatum2.ExitCode != 0)
			{
				Console.WriteLine(threadDatum2.ShaderFilename + "(1) : error compiling shader");
				Console.WriteLine(threadDatum2.StdError);
				Console.WriteLine(threadDatum2.StdOutput);
				flag = true;
			}
		}
		if (flag)
		{
			throw new Exception("Error encountered while compiling shaders.");
		}
	}

	public static List<string> SplitStringToList(string attrName, XmlElement root, XmlElement feature, string delimiter = ";")
	{
		List<string> list = new List<string>();
		string attribute = root.GetAttribute(attrName);
		if (string.IsNullOrEmpty(attribute) && feature != null)
		{
			attribute = feature.GetAttribute(attrName);
		}
		if (!string.IsNullOrEmpty(attribute))
		{
			list.AddRange(attribute.Split(delimiter.ToCharArray(), StringSplitOptions.RemoveEmptyEntries));
		}
		return list;
	}

	protected List<ShaderVersion> ExtractPossibleVersions(Type versionEnumeration, string opt, List<ShaderVersion> instances = null)
	{
		List<ShaderVersion> list = new List<ShaderVersion>();
		string[] array = ((opt == null) ? Enum.GetNames(versionEnumeration) : opt.Split(",".ToCharArray()));
		string[] array2 = array;
		foreach (string text in array2)
		{
			object obj = Enum.Parse(versionEnumeration, text);
			if (Enum.IsDefined(versionEnumeration, obj))
			{
				ShaderVersionAttribute attr = versionEnumeration.GetMember(obj.ToString())[0].GetCustomAttributes(typeof(ShaderVersionAttribute), inherit: false)[0] as ShaderVersionAttribute;
				ShaderVersion shaderVersion = null;
				if (instances != null)
				{
					shaderVersion = instances.Find((ShaderVersion v) => v.GetType() == attr.ShaderVersion);
				}
				if (shaderVersion == null)
				{
					ConstructorInfo[] constructors = attr.ShaderVersion.GetConstructors();
					List<object> list2 = new List<object>();
					list2.Add(this);
					foreach (ParameterInfo item in constructors[0].GetParameters().Skip(1))
					{
						list2.Add(item.DefaultValue);
					}
					shaderVersion = constructors[0].Invoke(list2.ToArray()) as ShaderVersion;
				}
				list.Add(shaderVersion);
				continue;
			}
			throw new Exception("Unknown shader version specified: " + text);
		}
		return list;
	}

	protected Uri GreatestCommonPath(ref string file0, ref string file1)
	{
		Uri uri = new Uri(Path.GetFullPath(file0));
		Uri uri2 = new Uri(Path.GetFullPath(file1));
		Uri uri3 = uri2.MakeRelativeUri(uri);
		string input = uri3.ToString();
		string text = Regex.Replace(input, "(?:[^/\\\\\\.]).*$", "");
		text = text.Replace('/', Path.DirectorySeparatorChar);
		Uri uri4 = new Uri(Path.Combine(Path.GetDirectoryName(uri2.LocalPath), text) + Path.DirectorySeparatorChar);
		file0 = uri4.MakeRelativeUri(uri).ToString();
		file1 = uri4.MakeRelativeUri(uri2).ToString();
		return uri4;
	}

	public void DeleteFileWildcard(string wildcardPath)
	{
		string directoryName = Path.GetDirectoryName(wildcardPath);
		if (!Directory.Exists(directoryName))
		{
			return;
		}
		string fileName = Path.GetFileName(wildcardPath);
		try
		{
			string[] files = Directory.GetFiles(directoryName, fileName);
			foreach (string text in files)
			{
				try
				{
					FileAttributes attributes = File.GetAttributes(text);
					attributes &= ~FileAttributes.ReadOnly;
					File.SetAttributes(text, attributes);
					File.Delete(text);
					if (CommandLineParser.GetOption<int>(CommandLineParser.Options.Verbosity) > 2)
					{
						Console.WriteLine("Deleted: {0}", text);
					}
				}
				catch (Exception ex)
				{
					Console.WriteLine("Could not delete file: " + ex.ToString());
				}
			}
		}
		catch (Exception ex2)
		{
			Console.WriteLine("Could not get files in directory: " + ex2.ToString());
		}
	}

	public void CopyFileWildcard(string wildcardSource, string destination)
	{
		string directoryName = Path.GetDirectoryName(wildcardSource);
		string fileName = Path.GetFileName(wildcardSource);
		try
		{
			string[] files = Directory.GetFiles(directoryName, fileName);
			foreach (string text in files)
			{
				try
				{
					string text2 = Path.Combine(destination, Path.GetFileName(text));
					File.Copy(text, text2);
					if (CommandLineParser.GetOption<int>(CommandLineParser.Options.Verbosity) > 2)
					{
						Console.WriteLine("Copied: {0} -> {1}", text, text2);
					}
				}
				catch (Exception ex)
				{
					Console.WriteLine("Could not copy file: " + ex.ToString());
				}
			}
		}
		catch (Exception ex2)
		{
			Console.WriteLine("Could not get files in directory: " + ex2.ToString());
		}
	}

	public virtual void ShaderBuildEvent(ShaderBuildEventType eventType)
	{
	}
}
