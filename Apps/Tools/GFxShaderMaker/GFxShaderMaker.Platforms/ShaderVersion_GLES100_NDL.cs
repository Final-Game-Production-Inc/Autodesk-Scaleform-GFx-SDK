using System.Text.RegularExpressions;

namespace GFxShaderMaker.Platforms;

public class ShaderVersion_GLES100_NDL : ShaderVersion_GLES100_Base
{
	public override string SourceExtension => ".gles2_noloop.glsl";

	public ShaderVersion_GLES100_NDL(ShaderPlatform platform)
		: base(platform, "GLES100_NDL")
	{
	}

	protected override void PerformVersionSpecificReplacements(ref string shaderCode, ShaderLinkedSource linkedSrc)
	{
		base.PerformVersionSpecificReplacements(ref shaderCode, linkedSrc);
		if (!Regex.IsMatch(shaderCode, "\\bfor\\b"))
		{
			return;
		}
		foreach (ShaderVariable item in linkedSrc.VariableList.FindAll((ShaderVariable v) => v.VarType == ShaderVariable.VariableType.Variable_Uniform))
		{
			string text;
			do
			{
				text = shaderCode;
				shaderCode = Regex.Replace(shaderCode, "(\\bfor\\b.*?[^%])(" + item.ID + "(?:\\.[xyzwrgba]))", "$1%$2%");
			}
			while (text != shaderCode);
		}
	}
}
