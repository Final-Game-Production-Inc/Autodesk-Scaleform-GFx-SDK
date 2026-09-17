using System;
using System.Collections.Generic;
using System.Xml;

namespace GFxShaderMaker;

public class ShaderFeatureFlavor : ShaderRestriction
{
	public string ID;

	public List<string> ExcludeIDs;

	public List<string> RequireIDs;

	public bool Hidden;

	public List<string> PostLink;

	public List<string> Flags;

	public static readonly string EmptyID = "Empty";

	public ShaderFeatureFlavor()
	{
		Flags = (PostLink = (RequireIDs = (ExcludeIDs = new List<string>())));
	}

	public void ReadFromXml(XmlElement root, XmlElement feature)
	{
		ExcludeIDs = new List<string>();
		ID = root.Attributes.GetNamedItem("id").Value;
		if (ID == EmptyID)
		{
			throw new Exception("'Empty' is an invalid identifier for a ShaderFeatureFlavor");
		}
		string attribute = root.GetAttribute("hide");
		if (string.IsNullOrEmpty(attribute))
		{
			attribute = feature.GetAttribute("hide");
		}
		if (!string.IsNullOrEmpty(attribute) && Convert.ToBoolean(attribute))
		{
			Hidden = true;
		}
		ExcludeIDs = ShaderPlatform.SplitStringToList("exclusive", root, feature);
		PostLink = ShaderPlatform.SplitStringToList("postlink", root, feature);
		Flags = ShaderPlatform.SplitStringToList("flag", root, feature);
		RequireIDs = ShaderPlatform.SplitStringToList("require", root, feature);
		base.ReadFromXml(root);
	}
}
