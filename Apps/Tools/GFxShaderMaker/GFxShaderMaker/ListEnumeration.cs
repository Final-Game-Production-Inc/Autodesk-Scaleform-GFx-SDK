using System;

namespace GFxShaderMaker;

public class ListEnumeration : CommandLineAction
{
	private string Header;

	private Type EnumType;

	public ListEnumeration(string header, Type enumType)
	{
		Header = header;
		EnumType = enumType;
	}

	public void Execute()
	{
		Console.WriteLine(Header);
		Console.WriteLine("-------------");
		string[] names = Enum.GetNames(EnumType);
		foreach (string arg in names)
		{
			Console.WriteLine(" {0,-10}", arg);
		}
		Console.WriteLine("");
	}
}
