using System.Collections.Generic;

namespace GFxShaderMaker.Platforms;

[Platform("GL", "OpenGL 2.0+")]
public class Platform_GL : Platform_GLCommon
{
	public enum CommandLineOptions
	{
		[CommandLineOption(CmdLineOptionType.OptionType_String, "glslversion", null, "The comma separated list of GLSL version(s) required (see -listversion).", null)]
		GLSLVersion,
		[CommandLineOption(CmdLineOptionType.OptionType_Flag, "enableUBO", null, "Enable generation of shaders compatible with UBOs (GLSL 1.50+ only).", null)]
		EnableUBO,
		[CommandLineOption(CmdLineOptionType.OptionType_Action, "listversion", null, "Lists the possible GLSL versions (use with -glslversion)", typeof(ListGLSLVersionsAction))]
		ListGLSLVersion
	}

	public enum GLSLVersions
	{
		[ShaderVersion(typeof(ShaderVersion_GLSL110))]
		GLSL110,
		[ShaderVersion(typeof(ShaderVersion_GLSL120))]
		GLSL120,
		[ShaderVersion(typeof(ShaderVersion_GLSL150))]
		GLSL150
	}

	private List<ShaderVersion> ReqShaderVersions;

	private List<ShaderVersion> PosShaderVersions;

	private List<ShaderVersion> ShaderVersions = new List<ShaderVersion>();

	public override List<ShaderVersion> PossibleShaderVersions
	{
		get
		{
			if (PosShaderVersions != null)
			{
				return PosShaderVersions;
			}
			PosShaderVersions = ExtractPossibleVersions(typeof(GLSLVersions), null);
			return PosShaderVersions;
		}
	}

	public override List<ShaderVersion> RequestedShaderVersions
	{
		get
		{
			if (ReqShaderVersions != null)
			{
				return ReqShaderVersions;
			}
			string option = CommandLineParser.GetOption<string>(CommandLineOptions.GLSLVersion.ToString());
			ReqShaderVersions = ExtractPossibleVersions(typeof(GLSLVersions), option, PossibleShaderVersions);
			return ReqShaderVersions;
		}
	}

	public Platform_GL()
	{
		ShaderVersions.Add(new ShaderVersion_GLSL110(this));
		ShaderVersions.Add(new ShaderVersion_GLSL120(this));
		ShaderVersions.Add(new ShaderVersion_GLSL150(this));
	}
}
