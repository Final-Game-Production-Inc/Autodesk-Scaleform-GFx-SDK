using System.Text.RegularExpressions;

namespace GFxShaderMaker.Platforms;

public abstract class ShaderVersion_GLESGLSL : ShaderVersion_GLSLCommon
{
	public ShaderVersion_GLESGLSL(ShaderPlatform platform, string id)
		: base(platform, id)
	{
	}

	protected override void PerformVersionSpecificReplacements(ref string shaderCode, ShaderLinkedSource linkedSrc)
	{
		base.PerformVersionSpecificReplacements(ref shaderCode, linkedSrc);
		shaderCode = Regex.Replace(shaderCode, "(^|\\b)half([1-4])x([1-4])\\b", "mediump mat$3");
		shaderCode = Regex.Replace(shaderCode, "(^|\\b)lowpf([1-4])x([1-4])", "lowp mat$3");
		shaderCode = Regex.Replace(shaderCode, "(^|\\b)half([1-4])\\b", "mediump vec$2");
		shaderCode = Regex.Replace(shaderCode, "(^|\\b)lowpf([1-4])\\b", "lowp vec$2");
		shaderCode = Regex.Replace(shaderCode, "(^|\\b)half\\b", "mediump float");
		shaderCode = Regex.Replace(shaderCode, "(^|\\b)lowpf\\b", "lowp float");
		shaderCode = Regex.Replace(shaderCode, "(^|\\b)lerp\\b", "mix");
		shaderCode = Regex.Replace(shaderCode, "(^|\\b)(?:float|half|lowpf)([1-4])x([1-4])\\b", "mat$3");
		shaderCode = Regex.Replace(shaderCode, "(^|\\b)(?:float|half|lowpf)([1-4])\\b", "vec$2");
		shaderCode = Regex.Replace(shaderCode, "(^|\\b)(?:float|half|lowpf)\\b", "float");
	}

	protected override string GetGLSLPrecisionString(ShaderLinkedSource linkedSrc)
	{
		if (linkedSrc.Pipeline.Type != ShaderPipeline.PipelineType.Fragment)
		{
			return "";
		}
		return "precision mediump float;\n";
	}
}
