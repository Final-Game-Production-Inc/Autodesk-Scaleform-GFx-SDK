namespace GFxShaderMaker.Platforms;

public class ShaderVersion_SM50 : ShaderVersion_SM40
{
	public ShaderVersion_SM50(ShaderPlatform platform, string id)
		: base(platform, id)
	{
		SupportedPipelines.Add(new ShaderPipeline(ShaderPipeline.PipelineType.Hull));
		SupportedPipelines.Add(new ShaderPipeline(ShaderPipeline.PipelineType.Domain));
		SupportedPipelines.Add(new ShaderPipeline(ShaderPipeline.PipelineType.Compute));
	}

	public override string GetShaderProfile(ShaderPipeline pipeline)
	{
		return pipeline.Type switch
		{
			ShaderPipeline.PipelineType.Hull => "hs_5_0", 
			ShaderPipeline.PipelineType.Domain => "ds_5_0", 
			ShaderPipeline.PipelineType.Fragment => "ps_5_0", 
			ShaderPipeline.PipelineType.Geometry => "gs_5_0", 
			ShaderPipeline.PipelineType.Compute => "cs_5_0", 
			_ => "vs_5_0", 
		};
	}
}
