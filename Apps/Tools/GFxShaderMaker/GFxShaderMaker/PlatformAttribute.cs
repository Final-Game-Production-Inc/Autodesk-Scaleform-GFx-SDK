using System;

namespace GFxShaderMaker;

[AttributeUsage(AttributeTargets.All)]
public class PlatformAttribute : Attribute
{
	public string Name { get; set; }

	public string Description { get; set; }

	public PlatformAttribute(string name, string description)
	{
		Name = name;
		Description = description;
	}
}
