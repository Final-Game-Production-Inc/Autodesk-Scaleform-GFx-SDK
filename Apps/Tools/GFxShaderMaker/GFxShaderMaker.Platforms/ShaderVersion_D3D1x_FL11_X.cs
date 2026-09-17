namespace GFxShaderMaker.Platforms;

internal class ShaderVersion_D3D1x_FL11_X : ShaderVersion_SM50
{
	public override string SourceExtension => ".fl11.hlsl";

	public ShaderVersion_D3D1x_FL11_X(ShaderPlatform platform)
		: base(platform, "D3D1xFL11X")
	{
	}
}
