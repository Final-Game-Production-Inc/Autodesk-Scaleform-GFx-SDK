using System;

namespace GFxShaderMaker;

[AttributeUsage(AttributeTargets.Field)]
public class ShaderVersionAttribute : Attribute
{
	public Type ShaderVersion { get; private set; }

	public ShaderVersionAttribute(Type ver)
	{
		ShaderVersion = ver;
	}
}
