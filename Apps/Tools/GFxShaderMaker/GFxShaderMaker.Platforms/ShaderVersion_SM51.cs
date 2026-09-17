namespace GFxShaderMaker.Platforms;

public class ShaderVersion_SM51 : ShaderVersion_SM50
{
	public ShaderVersion_SM51(ShaderPlatform platform, string id)
		: base(platform, id)
	{
	}

	public override string GetShaderProfile(ShaderPipeline pipeline)
	{
		return pipeline.Type switch
		{
			ShaderPipeline.PipelineType.Hull => "hs_5_1", 
			ShaderPipeline.PipelineType.Domain => "ds_5_1", 
			ShaderPipeline.PipelineType.Fragment => "ps_5_1", 
			ShaderPipeline.PipelineType.Geometry => "gs_5_1", 
			ShaderPipeline.PipelineType.Compute => "cs_5_1", 
			_ => "vs_5_1", 
		};
	}
}
