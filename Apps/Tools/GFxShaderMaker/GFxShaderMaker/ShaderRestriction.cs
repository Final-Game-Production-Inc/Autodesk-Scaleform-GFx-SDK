using System.Collections.Generic;
using System.Xml;

namespace GFxShaderMaker;

public class ShaderRestriction
{
	public List<string> Platforms;

	public List<string> Versions;

	public virtual void ReadFromXml(XmlElement root)
	{
		Platforms = ShaderPlatform.SplitStringToList("platform", root, null);
		Versions = ShaderPlatform.SplitStringToList("version", root, null);
	}

	public virtual bool IsRestricted(ShaderVersion ver)
	{
		if ((Platforms == null || Platforms.Count == 0) && (Versions == null || Versions.Count == 0))
		{
			return false;
		}
		if (Platforms.Contains(ver.Platform.PlatformName))
		{
			return false;
		}
		if (Versions.Contains(ver.ID))
		{
			return false;
		}
		return true;
	}
}
