using System;
using System.Threading;

namespace GFxShaderMaker;

internal class Program
{
	private static void Main(string[] args)
	{
		bool createdNew = false;
		new Mutex(initiallyOwned: true, "GFxShaderMaker", out createdNew);
		if (!createdNew)
		{
			try
			{
				Mutex.OpenExisting("GFxShaderMaker");
			}
			catch (Exception)
			{
			}
			Environment.Exit(0);
		}
		try
		{
			CommandLineParser.Initialize(args);
			Environment.Exit(0);
		}
		catch (Exception ex2)
		{
			ConsoleColor foregroundColor = Console.ForegroundColor;
			Console.ForegroundColor = ConsoleColor.Red;
			Console.WriteLine(ex2.Message);
			Console.ForegroundColor = ConsoleColor.Gray;
			Console.WriteLine(ex2.StackTrace);
			Console.ForegroundColor = foregroundColor;
			Environment.Exit(-1);
		}
	}
}
