using System.Collections.Generic;

namespace GFxShaderMaker.Platforms;

[Platform("XboxOneADK", "XboxOne HLSL (ADK)")]
public class Platform_XboxOneADK : Platform_D3D1x
{
	private List<ShaderVersion> ShaderVersions = new List<ShaderVersion>();

	protected override string D3DSDKEnvironmentVariable => "DurangoXDK";

	protected override bool PreferWindowsKitsFXC => false;

	public override string PlatformBase => "D3D1x";

	public override List<ShaderVersion> PossibleShaderVersions
	{
		get
		{
			if (PosShaderVersions != null)
			{
				return PosShaderVersions;
			}
			PosShaderVersions = new List<ShaderVersion>();
			PosShaderVersions.Add(new ShaderVersion_D3D1x_FL10_X(this));
			return PosShaderVersions;
		}
	}

	public override List<ShaderVersion> RequestedShaderVersions => PosShaderVersions;
}
