namespace GFxShaderMaker.Platforms;

internal class ShaderVersion_D3D9_SM20 : ShaderVersion_SM20
{
	public override string SourceExtension => ".sm20.hlsl";

	public ShaderVersion_D3D9_SM20(ShaderPlatform platform)
		: base(platform, "D3D9SM20")
	{
	}
}
