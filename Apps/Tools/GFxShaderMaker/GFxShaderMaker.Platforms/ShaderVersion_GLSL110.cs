using System;
using System.Collections.Generic;
using System.Text.RegularExpressions;

namespace GFxShaderMaker.Platforms;

public class ShaderVersion_GLSL110 : ShaderVersion_OpenGLGLSL
{
	private List<string> UnsupportedFlagsGLSL110 = new List<string>();

	public override string SourceExtension => ".110.glsl";

	protected override string GLSLVersionString => "";

	public override List<string> UnsupportedFlags => UnsupportedFlagsGLSL110;

	public ShaderVersion_GLSL110(ShaderPlatform platform)
		: base(platform, "GLSL110")
	{
		UnsupportedFlagsGLSL110.Add("Instanced");
	}

	protected override void PerformVersionSpecificReplacements(ref string shaderCode, ShaderLinkedSource linkedSrc)
	{
		base.PerformVersionSpecificReplacements(ref shaderCode, linkedSrc);
		foreach (ShaderVariable item in linkedSrc.VariableList.FindAll((ShaderVariable v) => v.VarType == ShaderVariable.VariableType.Variable_FragOut))
		{
			shaderCode = Regex.Replace(shaderCode, "\\b" + item.ID + "\\b", "gl_FragColor");
		}
	}

	protected override string GetShaderVariableQualifier(ShaderVariable.VariableType VarType, ShaderPipeline pipeline)
	{
		if (VarType == ShaderVariable.VariableType.Variable_FragOut)
		{
			return null;
		}
		return VarType.ToString().Split("_".ToCharArray(), StringSplitOptions.RemoveEmptyEntries)[1].ToLower();
	}
}
