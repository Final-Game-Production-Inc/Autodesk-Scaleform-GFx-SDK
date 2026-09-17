namespace GFxShaderMaker.Platforms;

internal class ShaderVersion_D3D1x_FL91 : ShaderVersion_SM20
{
	public override string SourceExtension => ".fl91.hlsl";

	public ShaderVersion_D3D1x_FL91(ShaderPlatform platform)
		: base(platform, "D3D1xFL91")
	{
	}

	public override string GetD3DFXCExtraOptions(string exe, ShaderLinkedSource src)
	{
		if (exe.Contains("Kits") || (src.ID != "FDrawableCopyPixels" && src.ID != "FDrawableCopyPixelsAlpha"))
		{
			return "/Gec";
		}
		return "/Gec /Od";
	}

	public override string GetShaderProfile(ShaderPipeline pipeline)
	{
		return pipeline.Type switch
		{
			ShaderPipeline.PipelineType.Fragment => "ps_4_0_level_9_1", 
			ShaderPipeline.PipelineType.Vertex => "vs_4_0_level_9_1", 
			_ => null, 
		};
	}

	public override void PostLink_Batch(ShaderLinkedSource linkedSrc)
	{
		base.PostLink_Batch(linkedSrc);
		ShaderVariable shaderVariable = linkedSrc.VariableList.Find((ShaderVariable v) => v.Semantic.StartsWith("INSTANCE"));
		if (shaderVariable != null)
		{
			linkedSrc.SourceCode = linkedSrc.SourceCode.Replace(shaderVariable.ID, shaderVariable.ID + " * 255.01f");
		}
	}
}
