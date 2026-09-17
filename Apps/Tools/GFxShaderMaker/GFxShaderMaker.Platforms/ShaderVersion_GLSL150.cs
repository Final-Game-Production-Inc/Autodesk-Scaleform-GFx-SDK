using System;
using System.Collections.Generic;
using System.Text.RegularExpressions;

namespace GFxShaderMaker.Platforms;

public class ShaderVersion_GLSL150 : ShaderVersion_OpenGLGLSL
{
	public override string SourceExtension => ".150.glsl";

	protected override string GLSLVersionString => "#version 150\n";

	public override bool UsesUniformBufferObjects => Convert.ToBoolean(CommandLineParser.GetOption<string>(Platform_GL.CommandLineOptions.EnableUBO.ToString()));

	public ShaderVersion_GLSL150(ShaderPlatform platform, string version = "GLSL150")
		: base(platform, version)
	{
	}

	protected override void PerformVersionSpecificReplacements(ref string shaderCode, ShaderLinkedSource linkedSrc)
	{
		base.PerformVersionSpecificReplacements(ref shaderCode, linkedSrc);
		shaderCode = Regex.Replace(shaderCode, "\\btexture2D\\b", "texture", RegexOptions.IgnoreCase);
		shaderCode = Regex.Replace(shaderCode, "\\btexture2DLod\\b", "textureLod", RegexOptions.IgnoreCase);
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

	protected override string AddShaderUniforms(ShaderLinkedSource linkedSrc, string shaderCode, List<ShaderVariable> uniforms)
	{
		if (!UsesUniformBufferObjects)
		{
			return base.AddShaderUniforms(linkedSrc, shaderCode, uniforms);
		}
		bool flag = false;
		string text = "";
		object obj = text;
		text = string.Concat(obj, "layout(std140) uniform ", linkedSrc.Pipeline.Letter, "Constants {\n");
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
}
