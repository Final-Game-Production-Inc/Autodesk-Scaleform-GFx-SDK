namespace GFxShaderMaker.Platforms;

public class ListGLSLVersionsAction : ListEnumeration
{
	public ListGLSLVersionsAction()
		: base("GLSL Versions:\n (use comma separated list if multiple are desired, eg. -glslversion GLSL110,GLSL150", typeof(Platform_GL.GLSLVersions))
	{
	}
}
