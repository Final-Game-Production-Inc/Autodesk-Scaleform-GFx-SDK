using System.Collections.Generic;

namespace GFxShaderMaker.Platforms;

[Platform("XboxOne", "XboxOne HLSL")]
public class Platform_XboxOne : Platform_D3D1x
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
			PosShaderVersions.Add(new ShaderVersion_D3D1x_FL11_X(this));
			return PosShaderVersions;
		}
	}

	public override List<ShaderVersion> RequestedShaderVersions => PossibleShaderVersions;
}
