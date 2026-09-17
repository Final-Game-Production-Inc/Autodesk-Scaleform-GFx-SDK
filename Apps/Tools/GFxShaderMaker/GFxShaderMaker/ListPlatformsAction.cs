using System;
using System.Linq;

namespace GFxShaderMaker;

public class ListPlatformsAction : CommandLineAction
{
	public void Execute()
	{
		Console.WriteLine("Platform List");
		Console.WriteLine("-------------");
		foreach (ShaderPlatform platform in ShaderPlatform.PlatformList)
		{
			PlatformAttribute[] array = platform.GetType().GetCustomAttributes(typeof(PlatformAttribute), inherit: true) as PlatformAttribute[];
			if (array.Count() > 0)
			{
				Console.WriteLine(" {0,-10} - {1}", array[0].Name, array[0].Description);
			}
		}
		Console.WriteLine("");
	}
}
