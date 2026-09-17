using System;
using System.Collections.Generic;

namespace GFxShaderMaker.Platforms;

[Platform("XboxOneD3D12", "XboxOne D3D12 HLSL 5.1")]
public class Platform_XboxOneD3D12 : Platform_D3D12
{
	private List<ShaderVersion> ShaderVersions = new List<ShaderVersion>();

	protected override string D3DSDKEnvironmentVariable => "DurangoXDK";

	protected override bool PreferWindowsKitsFXC => false;

	public override string PlatformBase => "D3D12";

	protected override string D3DFXCExtraOptions
	{
		get
		{
			string option = CommandLineParser.GetOption(CommandLineParser.Options.Config);
			switch (option)
			{
			case "Debug":
			case "DebugOpt":
				return "/Zi";
			case "Release":
				return "/Zi /O3";
			case "Shipping":
				return "/O3 /Qstrip_reflect /Qstrip_debug";
			default:
				throw new Exception("Unsupported configuration type: " + option);
			}
		}
	}

	public override List<ShaderVersion> PossibleShaderVersions
	{
		get
		{
			if (PosShaderVersions != null)
			{
				return PosShaderVersions;
			}
			PosShaderVersions = new List<ShaderVersion>();
			PosShaderVersions.Add(new ShaderVersion_D3D12(this));
			return PosShaderVersions;
		}
	}

	public override List<ShaderVersion> RequestedShaderVersions => PossibleShaderVersions;
}
