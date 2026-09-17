using System;
using System.Text.RegularExpressions;

namespace GFxShaderMaker.Platforms;

public class ShaderVersion_GLES100_Base : ShaderVersion_GLESGLSL
{
	public override string SourceExtension => ".gles2.glsl";

	protected override string GLSLVersionString => "";

	public override bool RequireShaderCombos => true;

	protected override string InstanceIDName => "gl_InstanceIDEXT";

	public ShaderVersion_GLES100_Base(ShaderPlatform platform, string versionName)
		: base(platform, versionName)
	{
	}

	public override string GetVariableUniformRegisterType(ShaderVariable var)
	{
		string result = "c";
		if (Regex.Matches(var.Type, "sampler", RegexOptions.IgnoreCase).Count > 0)
		{
			result = "s";
		}
		return result;
	}

	protected override string GetShaderVariableQualifier(ShaderVariable.VariableType VarType, ShaderPipeline pipeline)
	{
		if (VarType == ShaderVariable.VariableType.Variable_FragOut)
		{
			return null;
		}
		return VarType.ToString().Split("_".ToCharArray(), StringSplitOptions.RemoveEmptyEntries)[1].ToLower();
	}

	protected override string GetGLSLExtensionStrings(ShaderLinkedSource linkedSrc)
	{
		string text = "";
		if (linkedSrc.Flags.Contains("Derivatives"))
		{
			text += "#extension GL_OES_standard_derivatives : enable\n";
		}
		if (linkedSrc.Flags.Find((string f) => f == "Instanced") != null)
		{
			text += "#extension GL_EXT_draw_instanced : require\n";
		}
		return text;
	}

	protected override void PerformVersionSpecificReplacementSourceOnly(ref string sourceCode, ShaderLinkedSource linkedSrc)
	{
		base.PerformVersionSpecificReplacementSourceOnly(ref sourceCode, linkedSrc);
		ShaderVariable shaderVariable = null;
		if (linkedSrc.Pipeline.Type == ShaderPipeline.PipelineType.Fragment)
		{
			shaderVariable = linkedSrc.VariableList.FindLast((ShaderVariable v) => v.VarType == ShaderVariable.VariableType.Variable_FragOut);
			sourceCode = shaderVariable.Type + " " + shaderVariable.ID + ";\n" + sourceCode;
			sourceCode = sourceCode + "\ngl_FragColor = " + shaderVariable.ID + ";\n";
		}
	}

	protected override void PerformVersionSpecificReplacements(ref string shaderCode, ShaderLinkedSource linkedSrc)
	{
		base.PerformVersionSpecificReplacements(ref shaderCode, linkedSrc);
		shaderCode = Regex.Replace(shaderCode, "(^|\\b)texture2DLod\\s*\\((?'P0'" + ShaderVersion.SubexprRegex + "),(?'P1'" + ShaderVersion.SubexprRegex + "),(?'P2'" + ShaderVersion.SubexprRegex + ")\\)", "texture2D(${P0}, ${P1})", RegexOptions.IgnoreCase);
	}
}
