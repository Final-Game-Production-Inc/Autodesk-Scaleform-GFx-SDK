namespace GFxShaderMaker.Platforms;

public class ListGLESVersionsAction : ListEnumeration
{
	public ListGLESVersionsAction()
		: base("GLSL Versions:\n (use comma separated list if multiple are desired, eg. -glslversion GLES100,GLES300", typeof(Platform_GLES.GLSLVersions))
	{
	}
}
