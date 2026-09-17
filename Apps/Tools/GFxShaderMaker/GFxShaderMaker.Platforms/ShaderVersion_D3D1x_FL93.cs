namespace GFxShaderMaker.Platforms;

internal class ShaderVersion_D3D1x_FL93 : ShaderVersion_SM20
{
	public override string SourceExtension => ".fl93.hlsl";

	public ShaderVersion_D3D1x_FL93(ShaderPlatform platform)
		: base(platform, "D3D1xFL93")
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
			ShaderPipeline.PipelineType.Fragment => "ps_4_0_level_9_3", 
			ShaderPipeline.PipelineType.Vertex => "vs_4_0_level_9_3", 
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
