using System;
using System.Reflection;

namespace GFxShaderMaker;

public class HelpAction : CommandLineAction
{
	public void Execute()
	{
		Console.WriteLine("General Options:\n-------------------------\n");
		FieldInfo[] fields = typeof(CommandLineParser.Options).GetFields();
		foreach (FieldInfo fieldInfo in fields)
		{
			object[] customAttributes = fieldInfo.GetCustomAttributes(typeof(CommandLineOptionAttribute), inherit: true);
			for (int j = 0; j < customAttributes.Length; j++)
			{
				CommandLineOptionAttribute commandLineOptionAttribute = (CommandLineOptionAttribute)customAttributes[j];
				Console.WriteLine("-{0,-10} : {1}", commandLineOptionAttribute.CommandFlag, commandLineOptionAttribute.Description);
			}
		}
		string platformName = CommandLineParser.GetOption(CommandLineParser.Options.Platform);
		ShaderPlatform shaderPlatform = ShaderPlatform.PlatformList.Find((ShaderPlatform plt) => plt.PlatformName == platformName);
		if (shaderPlatform != null)
		{
			Type nestedType = shaderPlatform.GetType().GetNestedType("CommandLineOptions");
			if (nestedType != null && nestedType.IsEnum)
			{
				Console.WriteLine("\nPlatform Specific Options:\n-------------------------\n");
				FieldInfo[] fields2 = nestedType.GetFields();
				foreach (FieldInfo fieldInfo2 in fields2)
				{
					object[] customAttributes2 = fieldInfo2.GetCustomAttributes(typeof(CommandLineOptionAttribute), inherit: true);
					for (int num2 = 0; num2 < customAttributes2.Length; num2++)
					{
						CommandLineOptionAttribute commandLineOptionAttribute2 = (CommandLineOptionAttribute)customAttributes2[num2];
						Console.WriteLine("-{0,-10} : {1}", commandLineOptionAttribute2.CommandFlag, commandLineOptionAttribute2.Description);
					}
				}
			}
		}
		else
		{
			Console.WriteLine("\nNo -platform specified. Use -platform with -help to get platform specific options.");
		}
		Console.WriteLine("");
	}
}
