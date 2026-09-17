namespace GFxShaderMaker.Platforms;

internal class ShaderVersion_D3D9_SM30 : ShaderVersion_SM30
{
	public override string SourceExtension => ".sm30.hlsl";

	public ShaderVersion_D3D9_SM30(ShaderPlatform platform)
		: base(platform, "D3D9SM30")
	{
	}
}
