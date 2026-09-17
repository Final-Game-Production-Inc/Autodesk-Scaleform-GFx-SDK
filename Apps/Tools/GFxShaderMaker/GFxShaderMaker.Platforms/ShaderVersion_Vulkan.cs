using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace GFxShaderMaker.Platforms;

public class ShaderVersion_Vulkan : ShaderVersion_GLSL150
{
	private List<string> UnsupportedFlagsVulkan = new List<string>();

	public override string SourceExtension => ".glsl";

	protected override string GLSLVersionString => "#version 450\n";

	protected override string InstanceIDName => "gl_InstanceIndex";

	protected override string BatchIndexType => "uint";

	public override bool RequireShaderCombos => true;

	public override List<string> UnsupportedFlags => UnsupportedFlagsVulkan.Union(base.UnsupportedFlags).ToList();

	public override bool UsesUniformBufferObjects => true;

	public ShaderVersion_Vulkan(ShaderPlatform platform, string version = "Vulkan")
		: base(platform, version)
	{
	}

	protected override string GetGLSLExtensionStrings(ShaderLinkedSource linkedSrc)
	{
		string text = "";
		return text + "#extension GL_ARB_shading_language_420pack : enable\n";
	}

	public override string GetShaderFilename(ShaderLinkedSource src)
	{
		string text = ".unknown";
		return string.Concat(str3: src.Pipeline.Type switch
		{
			ShaderPipeline.PipelineType.Vertex => ".vert", 
			ShaderPipeline.PipelineType.Fragment => ".frag", 
			ShaderPipeline.PipelineType.Geometry => ".geom", 
			ShaderPipeline.PipelineType.Compute => ".comp", 
			_ => throw new Exception($"Unexpected shader pipeline: {src.Pipeline.Name}"), 
		}, str0: base.ID, str1: "_", str2: src.ID);
	}

	private void WriteBinaryDataToCArray(ShaderLinkedSource src, byte[] inputData, StreamWriter outputStream)
	{
		string text = base.ID + "_" + src.ID;
		string text2 = "extern const unsigned char pBinary_" + text + "[] = {";
		int num = 0;
		foreach (byte b in inputData)
		{
			if (num % 32 == 0)
			{
				num = 0;
				text2 += "\n    ";
			}
			text2 = text2 + "0x" + b.ToString("X2") + ",";
			num++;
		}
		text2 += "};\n";
		object obj = text2;
		text2 = string.Concat(obj, "extern const int pBinary_", text, "_size = ", inputData.Count(), ";\n");
		outputStream.Write(text2);
	}

	public override void WriteBinaryShaderSource(StreamWriter sourceFile)
	{
		foreach (ShaderLinkedSource value in LinkedSourceDuplicates.Values)
		{
			string path = Path.Combine(base.SourceDirectory, GetShaderFilename(value)) + ".spv";
			byte[] inputData = File.ReadAllBytes(path);
			WriteBinaryDataToCArray(value, inputData, sourceFile);
		}
	}

	protected override string AddShaderUniforms(ShaderLinkedSource linkedSrc, string shaderCode, List<ShaderVariable> uniforms)
	{
		bool flag = false;
		string text = "";
		object obj = text;
		text = string.Concat(obj, "layout(set=0, binding = ", Convert.ToInt32(linkedSrc.Pipeline.Type), ") uniform ", linkedSrc.Pipeline.Letter, "Constants {\n");
		foreach (ShaderVariable uniform in uniforms)
		{
			string shaderVariableQualifier = GetShaderVariableQualifier(uniform.VarType, linkedSrc.Pipeline);
			if (shaderVariableQualifier == "uniform" && !uniform.SamplerType)
			{
				flag = true;
				string text2 = text;
				text = text2 + shaderVariableQualifier + " " + uniform.Type + " " + uniform.ID + ((uniform.ArraySize > 1) ? ("[" + uniform.ArraySize + "]") : "") + ";\n";
			}
		}
		text += "};\n";
		if (flag)
		{
			shaderCode += text;
		}
		int num = 0;
		foreach (ShaderVariable sortedAttribute in linkedSrc.SortedAttributeList)
		{
			string shaderVariableQualifier2 = GetShaderVariableQualifier(sortedAttribute.VarType, linkedSrc.Pipeline);
			object obj2 = shaderCode;
			shaderCode = string.Concat(obj2, "layout (location = ", num, ") ", shaderVariableQualifier2, " ", sortedAttribute.Type, " ", sortedAttribute.ID, (sortedAttribute.ArraySize > 1) ? ("[" + sortedAttribute.ArraySize + "]") : "", ";\n");
			num++;
		}
		Dictionary<string, int> dictionary = new Dictionary<string, int>();
		foreach (ShaderVariable uniform2 in uniforms)
		{
			string shaderVariableQualifier3 = GetShaderVariableQualifier(uniform2.VarType, linkedSrc.Pipeline);
			if ((shaderVariableQualifier3 != "uniform" && uniform2.VarType != ShaderVariable.VariableType.Variable_Attribute) || uniform2.SamplerType)
			{
				int num2 = 0;
				string text3 = (uniform2.SamplerType ? "binding" : "location");
				if (!dictionary.ContainsKey(shaderVariableQualifier3))
				{
					num2 = (uniform2.SamplerType ? 2 : 0);
					dictionary.Add(shaderVariableQualifier3, num2);
				}
				else
				{
					num2 = ++dictionary[shaderVariableQualifier3];
				}
				object obj3 = shaderCode;
				shaderCode = string.Concat(obj3, "layout (", text3, " = ", num2, ") ", shaderVariableQualifier3, " ", uniform2.Type, " ", uniform2.ID, (uniform2.ArraySize > 1) ? ("[" + uniform2.ArraySize + "]") : "", ";\n");
			}
		}
		return shaderCode;
	}
}
