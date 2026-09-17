namespace GFxShaderMaker.Platforms;

internal class ShaderVersion_D3D1x_FL10_X : ShaderVersion_SM40
{
	public override string SourceExtension => ".fl10.hlsl";

	public ShaderVersion_D3D1x_FL10_X(ShaderPlatform platform)
		: base(platform, "D3D1xFL10X")
	{
	}
}
