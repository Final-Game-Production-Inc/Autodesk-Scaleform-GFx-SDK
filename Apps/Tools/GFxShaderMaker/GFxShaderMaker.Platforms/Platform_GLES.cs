using System.Collections.Generic;

namespace GFxShaderMaker.Platforms;

[Platform("GLES", "GLES 2.0+")]
public class Platform_GLES : Platform_GLCommon
{
	public enum CommandLineOptions
	{
		[CommandLineOption(CmdLineOptionType.OptionType_String, "glslversion", null, "The comma separated list of GLSL version(s) required (see -listversion).", null)]
		GLSLVersion,
		[CommandLineOption(CmdLineOptionType.OptionType_Flag, "enableUBO", null, "Enable generation of shaders compatible with UBOs (GLSL ES 3.0+).", null)]
		EnableUBO,
		[CommandLineOption(CmdLineOptionType.OptionType_Action, "listversion", null, "Lists the possible GLSL versions (use with -glslversion)", typeof(ListGLESVersionsAction))]
		ListGLSLVersion
	}

	public enum GLSLVersions
	{
		[ShaderVersion(typeof(ShaderVersion_GLES100_NDL))]
		GLES100_NoLoop,
		[ShaderVersion(typeof(ShaderVersion_GLES100))]
		GLES100,
		[ShaderVersion(typeof(ShaderVersion_GLES300))]
		GLES300
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

	public override string PlatformBase => "GL";

	public Platform_GLES()
	{
		ShaderVersions.Add(new ShaderVersion_GLES100_NDL(this));
		ShaderVersions.Add(new ShaderVersion_GLES100(this));
		ShaderVersions.Add(new ShaderVersion_GLES300(this));
	}
}
