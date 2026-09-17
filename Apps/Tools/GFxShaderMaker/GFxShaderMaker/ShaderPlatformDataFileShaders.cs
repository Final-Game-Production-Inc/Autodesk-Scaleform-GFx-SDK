namespace GFxShaderMaker;

public abstract class ShaderPlatformDataFileShaders : ShaderPlatform
{
	protected override void writeHeaderPipelineShaderDataMembers(IndentStreamWriter headerFile, ShaderPipeline pipeline)
	{
	}

	protected override void writeSourcePipelineShaderGlobals(IndentStreamWriter sourceFile, ShaderVersion ver, ShaderPipeline pipeline, string srcID)
	{
	}

	protected override void writeSourcePipelineShaderData(IndentStreamWriter sourceFile, ShaderVersion ver, ShaderPipeline pipeline, ShaderLinkedSource src)
	{
	}
}
