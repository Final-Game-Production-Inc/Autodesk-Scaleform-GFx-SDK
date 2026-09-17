namespace GFxShaderMaker.Platforms;

public class ListD3D9ShaderModelsAction : ListEnumeration
{
	public ListD3D9ShaderModelsAction()
		: base("D3D9 Shader Models:\n (use comma separated list if multiple are desired, eg. -shadermodel SM20,SM30", typeof(Platform_D3D9.ShaderModels))
	{
	}
}
