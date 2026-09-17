using System;
using System.Collections.Generic;
using System.Text.RegularExpressions;

namespace GFxShaderMaker.Platforms;

public class ShaderVersion_GLES300 : ShaderVersion_GLESGLSL
{
	public override string SourceExtension => ".gles3.glsl";

	protected override string GLSLVersionString => "#version 300 es\n";

	public override bool UsesUniformBufferObjects => Convert.ToBoolean(CommandLineParser.GetOption<string>(Platform_GLES.CommandLineOptions.EnableUBO.ToString()));

	protected override string InstanceIDName => "gl_InstanceID";

	public ShaderVersion_GLES300(ShaderPlatform platform)
		: base(platform, "GLES30")
	{
	}

	protected override string GetShaderVariableQualifier(ShaderVariable.VariableType VarType, ShaderPipeline pipeline)
	{
		switch (VarType)
		{
		case ShaderVariable.VariableType.Variable_Uniform:
			return "uniform";
		case ShaderVariable.VariableType.Variable_Attribute:
			return "in";
		case ShaderVariable.VariableType.Variable_Varying:
			if (pipeline.Type != ShaderPipeline.PipelineType.Fragment)
			{
				return "out";
			}
			return "in";
		case ShaderVariable.VariableType.Variable_FragOut:
			return "out";
		default:
			throw new Exception("GetShaderVariableQualifier should never be called with type: " + VarType);
		}
	}

	protected override string GetGLSLExtensionStrings(ShaderLinkedSource linkedSrc)
	{
		return "";
	}

	protected override string AddShaderUniforms(ShaderLinkedSource linkedSrc, string shaderCode, List<ShaderVariable> uniforms)
	{
		if (!UsesUniformBufferObjects)
		{
			return base.AddShaderUniforms(linkedSrc, shaderCode, uniforms);
		}
		bool flag = false;
		string text = "";
		object obj = text;
		text = string.Concat(obj, "uniform ", linkedSrc.Pipeline.Letter, "Constants {\n");
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
		foreach (ShaderVariable uniform2 in uniforms)
		{
			string shaderVariableQualifier2 = GetShaderVariableQualifier(uniform2.VarType, linkedSrc.Pipeline);
			if (shaderVariableQualifier2 != "uniform" || uniform2.SamplerType)
			{
				string text3 = shaderCode;
				shaderCode = text3 + shaderVariableQualifier2 + " " + uniform2.Type + " " + uniform2.ID + ((uniform2.ArraySize > 1) ? ("[" + uniform2.ArraySize + "]") : "") + ";\n";
			}
		}
		return shaderCode;
	}

	protected override void PerformVersionSpecificReplacements(ref string sourceCode, ShaderLinkedSource linkedSrc)
	{
		base.PerformVersionSpecificReplacements(ref sourceCode, linkedSrc);
		sourceCode = Regex.Replace(sourceCode, "\\btexture2D\\b", "texture", RegexOptions.IgnoreCase);
		sourceCode = Regex.Replace(sourceCode, "\\btexture2DLod\\b", "textureLod", RegexOptions.IgnoreCase);
	}
}
