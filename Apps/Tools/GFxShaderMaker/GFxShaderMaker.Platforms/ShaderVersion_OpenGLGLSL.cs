using System.Text.RegularExpressions;

namespace GFxShaderMaker.Platforms;

public abstract class ShaderVersion_OpenGLGLSL : ShaderVersion_GLSLCommon
{
	protected override string InstanceIDName => "gl_InstanceID";

	public ShaderVersion_OpenGLGLSL(ShaderPlatform platform, string id)
		: base(platform, id)
	{
	}

	protected override void PerformVersionSpecificReplacements(ref string shaderCode, ShaderLinkedSource linkedSrc)
	{
		base.PerformVersionSpecificReplacements(ref shaderCode, linkedSrc);
		shaderCode = Regex.Replace(shaderCode, "\\blerp\\b", "mix");
		shaderCode = Regex.Replace(shaderCode, "\\b(?:float|half|lowpf)([1-4])x([1-4])\\b", "mat$2");
		shaderCode = Regex.Replace(shaderCode, "\\b(?:float|half|lowpf)([1-4])\\b", "vec$1");
		shaderCode = Regex.Replace(shaderCode, "\\b(?:float|half|lowpf)\\b", "float");
	}

	protected override string GetGLSLExtensionStrings(ShaderLinkedSource linkedSrc)
	{
		string text = "";
		if (linkedSrc.Flags.Find((string f) => f == "Instanced") != null)
		{
			text += "#extension GL_ARB_draw_instanced : enable\n";
		}
		return text;
	}
}
