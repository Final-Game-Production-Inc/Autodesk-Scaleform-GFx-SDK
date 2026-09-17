using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text.RegularExpressions;

namespace GFxShaderMaker.Platforms;

[Platform("Vulkan", "GLSL (converted to SPIR-V)")]
public class Platform_Vulkan : ShaderPlatformBinaryShaders
{
	private List<ShaderVersion> ShaderVersions = new List<ShaderVersion>();

	private Dictionary<int, string> mVertexBindingDictionary = new Dictionary<int, string>();

	private Dictionary<int, string> mVertexAttributeDictionary = new Dictionary<int, string>();

	public string VKSDKEnvironmentVariable => "VK_SDK_PATH";

	public string VKShaderCompilerExecutable => "glslangValidator.exe";

	public override string PlatformBinarySourceFilename
	{
		get
		{
			string option = CommandLineParser.GetOption(CommandLineParser.Options.OutputDirectory);
			return Path.Combine(option, base.PlatformName + "_ShaderBinary.cpp");
		}
	}

	public override List<ShaderVersion> RequestedShaderVersions => ShaderVersions;

	public Platform_Vulkan()
	{
		ShaderVersions.Add(new ShaderVersion_Vulkan(this));
	}

	public override void CreateShaderOutput()
	{
		string environmentVariable = Environment.GetEnvironmentVariable(VKSDKEnvironmentVariable);
		if (string.IsNullOrEmpty(environmentVariable))
		{
			throw new Exception($"Environment variable {VKSDKEnvironmentVariable} not found.");
		}
		if (!Directory.Exists(environmentVariable))
		{
			throw new Exception($"({VKSDKEnvironmentVariable} = {environmentVariable}) directory does not exist.");
		}
		IEnumerable<string> files = Directory.GetFiles(environmentVariable, VKShaderCompilerExecutable, SearchOption.AllDirectories);
		if (files.Count() == 0)
		{
			throw new Exception($"({VKSDKEnvironmentVariable} = {environmentVariable}) could not locate {VKShaderCompilerExecutable}.");
		}
		if (files.Count() > 1 && CommandLineParser.GetOption<int>(CommandLineParser.Options.Verbosity) > 1)
		{
			Console.WriteLine("Multiple {0} executables found. Using the first one ({1}).", VKShaderCompilerExecutable, files.First());
		}
		string text = files.First();
		if (CommandLineParser.GetOption<int>(CommandLineParser.Options.Verbosity) > 1)
		{
			Console.WriteLine("Using {0} to compile shaders.", text);
		}
		if (!Directory.Exists(PlatformObjDirectory))
		{
			Directory.CreateDirectory(PlatformObjDirectory);
		}
		List<CompileThreadData> list = new List<CompileThreadData>();
		foreach (ShaderVersion requestedShaderVersion in RequestedShaderVersions)
		{
			foreach (ShaderLinkedSource value in requestedShaderVersion.LinkedSourceDuplicates.Values)
			{
				CompileThreadData item = new CompileThreadData(this, requestedShaderVersion, value, text);
				list.Add(item);
			}
		}
		CompileShadersThreaded(list);
		CreateBinarySource();
	}

	protected override string GetBinaryShaderDeclaration(ShaderPipeline pipeline)
	{
		return "const unsigned char *   pBinary;\nint                     BinarySize;";
	}

	protected override string GetBinaryShaderReference(ShaderPipeline pipeline, string id)
	{
		return string.Format("pBinary_{0},\npBinary_{0}_size,", id);
	}

	protected override string GetBinaryShaderExtern(ShaderPipeline pipeline, string id)
	{
		return string.Format("extern const unsigned char pBinary_{0}[];\nextern const int           pBinary_{0}_size;", id);
	}

	protected override void CompileSingleShaderImpl(CompileThreadData ctdata)
	{
		if (ctdata != null)
		{
			Platform_Vulkan platform_Vulkan = ctdata.This as Platform_Vulkan;
			ShaderVersion sVersion = ctdata.SVersion;
			ShaderLinkedSource source = ctdata.Source;
			string exe = ctdata.Exe;
			string text = Path.Combine(ctdata.SVersion.SourceDirectory, sVersion.GetShaderFilename(source));
			if (!File.Exists(text))
			{
				throw new Exception("Expected to find " + text + " shader source, but it did not exist.");
			}
			string path = text + ".spv";
			string text2 = Path.Combine(platform_Vulkan.PlatformObjDirectory, path);
			if (!Directory.Exists(Path.GetDirectoryName(Path.GetDirectoryName(platform_Vulkan.PlatformObjDirectory))))
			{
				Directory.CreateDirectory(Path.GetDirectoryName(platform_Vulkan.PlatformObjDirectory));
			}
			string text3 = "-V -o \"" + text2 + "\" \"" + text + "\"";
			ctdata.ExitCode = launchProcess(exe, text3, out ctdata.StdOutput, out ctdata.StdError);
			ctdata.ShaderFilename = text;
			ctdata.CommandLine = exe + " " + text3;
		}
	}

	protected ulong GenerateVertexBindingDefinition(ShaderLinkedSource src, ref string vertexBindingString, ref Dictionary<int, string> vertexBindingDictionary)
	{
		List<ShaderVariable> sortedAttributeList = src.SortedAttributeList;
		long num = 0L;
		int elementSize = 0;
		string format = "";
		foreach (ShaderVariable item in sortedAttributeList)
		{
			GetVkFormatAndElementSize(item, ref format, ref elementSize);
			num += elementSize;
		}
		vertexBindingString = "{{ 0, " + num + ", VK_VERTEX_INPUT_RATE_VERTEX }}";
		int hashCode = vertexBindingString.GetHashCode();
		if (!vertexBindingDictionary.ContainsKey(hashCode))
		{
			vertexBindingDictionary.Add(hashCode, vertexBindingString);
		}
		else
		{
			vertexBindingString = null;
		}
		return Convert.ToUInt64(hashCode + uint.MaxValue);
	}

	protected ulong GenerateDescriptorSetLayoutBinding(List<ShaderLinkedSource> srcList, ref string layoutBindings, ref int layoutBindingCount, ref Dictionary<int, string> layoutBindingDictionary)
	{
		layoutBindings = "{\n";
		layoutBindingCount = 0;
		srcList.Sort((ShaderLinkedSource s0, ShaderLinkedSource s1) => s0.Pipeline.Type.CompareTo(s1.Pipeline.Type));
		foreach (ShaderLinkedSource src in srcList)
		{
			string text = "VK_SHADER_STAGE_" + src.Pipeline.Type.ToString().ToUpper() + "_BIT";
			object obj = layoutBindings;
			layoutBindings = string.Concat(obj, "{", layoutBindingCount, ", VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, ", text, ", 0},\n");
			layoutBindingCount++;
			foreach (ShaderVariable item in src.VariableList.FindAll((ShaderVariable v) => v.SamplerType))
			{
				object obj2 = layoutBindings;
				layoutBindings = string.Concat(obj2, "{", layoutBindingCount, ", VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, ", item.ArraySize, ",", text, ", 0},\n");
				layoutBindingCount++;
			}
		}
		layoutBindings += "};";
		int hashCode = layoutBindings.GetHashCode();
		if (!layoutBindingDictionary.ContainsKey(hashCode))
		{
			layoutBindingDictionary.Add(hashCode, layoutBindings);
		}
		else
		{
			layoutBindings = null;
		}
		return Convert.ToUInt64(hashCode + uint.MaxValue);
	}

	protected ulong GenerateDescriptorSetLayout(int layoutBindingCount, ulong descriptorLayoutBindingHash, ref string setLayout, ref Dictionary<int, string> layoutDictionary)
	{
		setLayout = "{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, 0, 0, " + layoutBindingCount + ", DescriptorSetLayoutBinding_" + descriptorLayoutBindingHash + "};\n";
		int hashCode = setLayout.GetHashCode();
		if (!layoutDictionary.ContainsKey(hashCode))
		{
			layoutDictionary.Add(hashCode, setLayout);
		}
		else
		{
			setLayout = null;
		}
		return Convert.ToUInt64(hashCode + uint.MaxValue);
	}

	private void GetVkFormatAndElementSize(ShaderVariable var, ref string format, ref int elementSize)
	{
		string semantic = var.Semantic;
		semantic = Regex.Replace(semantic, "\\d+$", "");
		switch (semantic)
		{
		default:
			throw new Exception("Unexpected semantic: " + semantic);
		case "POSITION":
			format = "VK_FORMAT_R32G32_SFLOAT";
			elementSize = 8;
			break;
		case "COLOR":
			format = "VK_FORMAT_R8G8B8A8_UNORM";
			elementSize = 4;
			break;
		case "TEXCOORD":
			format = "VK_FORMAT_R32G32_SFLOAT";
			elementSize = 8;
			break;
		case "INSTANCE":
			format = "VK_FORMAT_R8_UINT";
			elementSize = 4;
			break;
		case "FACTOR":
			format = "VK_FORMAT_R8G8B8A8_UNORM";
			elementSize = 4;
			break;
		}
	}

	protected ulong GenerateVertexAttributeDefinition(ShaderLinkedSource src, ref string vertexAttributeString, ref Dictionary<int, string> vertexAttributeDictionary)
	{
		List<ShaderVariable> sortedAttributeList = src.SortedAttributeList;
		int num = 0;
		int num2 = 0;
		string format = "";
		int num3 = 0;
		vertexAttributeString = "{";
		foreach (ShaderVariable item in sortedAttributeList)
		{
			int elementSize = 0;
			GetVkFormatAndElementSize(item, ref format, ref elementSize);
			object obj = vertexAttributeString;
			vertexAttributeString = string.Concat(obj, "{", num, ", ", num2, ", ", format, ", ", num3, "},\n");
			num3 += elementSize;
			num2 = 0;
			num++;
		}
		vertexAttributeString += "}";
		int hashCode = vertexAttributeString.GetHashCode();
		if (!vertexAttributeDictionary.ContainsKey(hashCode))
		{
			vertexAttributeDictionary.Add(hashCode, vertexAttributeString);
		}
		else
		{
			vertexAttributeString = null;
		}
		return Convert.ToUInt64(hashCode + uint.MaxValue);
	}

	protected override void writeHeaderShaderDescFunctions(IndentStreamWriter headerFile)
	{
		base.writeHeaderShaderDescFunctions(headerFile);
		headerFile.Write("static const VkDescriptorSetLayoutCreateInfo* GetDescriptorSetLayoutCreateInfo(unsigned comboIndex, ShaderVersion ver = ShaderVersion_Default);\n");
	}

	protected override void writeSourceShaderDescFunctions(IndentStreamWriter sourceFile)
	{
		base.writeSourceShaderDescFunctions(sourceFile);
		Dictionary<int, string> layoutBindingDictionary = new Dictionary<int, string>();
		Dictionary<int, string> layoutDictionary = new Dictionary<int, string>();
		Dictionary<uint, ulong> dictionary = new Dictionary<uint, ulong>();
		foreach (ShaderVersion requestedShaderVersion in RequestedShaderVersions)
		{
			uint num = requestedShaderVersion.LinkedSources.Max((ShaderLinkedSource s) => s.ShaderComboIndex);
			uint comboIndex;
			for (comboIndex = 0u; comboIndex < num; comboIndex++)
			{
				List<ShaderLinkedSource> list = requestedShaderVersion.LinkedSources.FindAll((ShaderLinkedSource s) => s.ShaderComboIndex == comboIndex);
				if (list.Count != 0)
				{
					string layoutBindings = "";
					string setLayout = "";
					int layoutBindingCount = 0;
					ulong num2 = GenerateDescriptorSetLayoutBinding(list, ref layoutBindings, ref layoutBindingCount, ref layoutBindingDictionary);
					ulong num3 = GenerateDescriptorSetLayout(layoutBindingCount, num2, ref setLayout, ref layoutDictionary);
					if (layoutBindings != null)
					{
						sourceFile.Write("VkDescriptorSetLayoutBinding DescriptorSetLayoutBinding_" + num2 + "[" + layoutBindingCount + "] = " + layoutBindings + ";\n");
					}
					if (setLayout != null)
					{
						sourceFile.Write("VkDescriptorSetLayoutCreateInfo DescriptorSetCreateInfo_" + num3 + " = " + setLayout + "\n");
					}
					dictionary.Add(comboIndex, num3);
				}
			}
		}
		sourceFile.Write("const VkDescriptorSetLayoutCreateInfo* ShaderDesc::GetDescriptorSetLayoutCreateInfo(unsigned comboIndex, ShaderVersion ver)\n");
		sourceFile.Write("{\n");
		sourceFile.Write("switch(ver)\n");
		sourceFile.Write("{\n");
		foreach (ShaderVersion requestedShaderVersion2 in RequestedShaderVersions)
		{
			sourceFile.Write("case ShaderVersion_" + requestedShaderVersion2.ID + ":\n");
			sourceFile.Write("switch(comboIndex)\n");
			sourceFile.Write("{\n");
			uint num4 = requestedShaderVersion2.LinkedSources.Max((ShaderLinkedSource s) => s.ShaderComboIndex);
			for (uint num5 = 0u; num5 < num4; num5++)
			{
				if (dictionary.ContainsKey(num5))
				{
					sourceFile.Write("case " + num5 + ": return &DescriptorSetCreateInfo_" + dictionary[num5] + ";\n");
				}
			}
			sourceFile.Write("default: SF_DEBUG_ASSERT1(0, \"Invalid shader combo index provided (%d)\", comboIndex);\nbreak;\n;");
			sourceFile.Write("break;\n");
			sourceFile.Write("}\n");
		}
		sourceFile.Write("default: SF_DEBUG_ASSERT1(0, \"Invalid shader platform provided (%d)\", ver); return 0;\n");
		sourceFile.Write("}\n");
		sourceFile.Write("}\n");
	}

	protected override uint writeSourcePipelineShaderDescs(ShaderPipeline pipeline, IndentStreamWriter sourceFile, Dictionary<int, string> uniformDefDictionary, uint shadowOffsetStart, Dictionary<int, string> batchUniformDefDictionary)
	{
		if (pipeline.Type == ShaderPipeline.PipelineType.Vertex)
		{
			foreach (ShaderVersion requestedShaderVersion in RequestedShaderVersions)
			{
				new List<ShaderLinkedSource>();
				foreach (List<ShaderLinkedSource> value in requestedShaderVersion.LinkedSourceUniqueDescs.Values)
				{
					if (value.Count > 0 && value.First().Pipeline.Type == pipeline.Type)
					{
						string vertexBindingString = null;
						string vertexAttributeString = null;
						ulong num = GenerateVertexBindingDefinition(value.First(), ref vertexBindingString, ref mVertexBindingDictionary);
						ulong num2 = GenerateVertexAttributeDefinition(value.First(), ref vertexAttributeString, ref mVertexAttributeDictionary);
						if (vertexBindingString != null)
						{
							sourceFile.Write("VkVertexInputBindingDescription VertexBinding_" + num + "[] = " + vertexBindingString + ";\n");
						}
						if (vertexAttributeString != null)
						{
							sourceFile.Write("VkVertexInputAttributeDescription VertexAttributes_" + num2 + "[] = \n" + vertexAttributeString + ";\n");
						}
					}
				}
			}
		}
		return base.writeSourcePipelineShaderDescs(pipeline, sourceFile, uniformDefDictionary, shadowOffsetStart, batchUniformDefDictionary);
	}

	public override string GeneratePipelineHeaderExtras(ShaderPipeline pipeline)
	{
		string text = base.GeneratePipelineHeaderExtras(pipeline);
		string text2 = "const char* ShaderName;\n";
		if (pipeline.Type == ShaderPipeline.PipelineType.Vertex)
		{
			text2 += "VkPipelineVertexInputStateCreateInfo VertexInputCreateInfo;\n";
		}
		return text + text2;
	}

	public override string GeneratePipelineSourceExtras(ShaderVersion ver, ShaderPipeline pipeline, ShaderLinkedSource src)
	{
		string text = base.GeneratePipelineSourceExtras(ver, pipeline, src);
		string text2 = "/* ShaderName */          \"" + src.SourceCodeDuplicateID + "\",\n";
		if (pipeline.Type == ShaderPipeline.PipelineType.Vertex)
		{
			List<ShaderVariable> sortedAttributeList = src.SortedAttributeList;
			string vertexBindingString = null;
			string vertexAttributeString = null;
			ulong num = GenerateVertexBindingDefinition(src, ref vertexBindingString, ref mVertexBindingDictionary);
			ulong num2 = GenerateVertexAttributeDefinition(src, ref vertexAttributeString, ref mVertexAttributeDictionary);
			object obj = text2;
			text2 = string.Concat(obj, "/* VertexInputCreateInfo*/ { \nVK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, // sType \nnullptr, 0, 1, VertexBinding_", num, ",\n", sortedAttributeList.Count, ", VertexAttributes_", num2, " }\n");
		}
		return text + text2;
	}

	protected override void writeHeaderPreamble(IndentStreamWriter headerFile)
	{
		headerFile.Write("#include \"Vulkan_Common.h\" // include vulkan.h for Vulkan structs\n");
		base.writeHeaderPreamble(headerFile);
	}
}
