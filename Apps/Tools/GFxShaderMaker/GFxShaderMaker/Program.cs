using System;
using System.Runtime.InteropServices;
using System.Threading;

namespace GFxShaderMaker;

internal class Program
{
	// Without these flags a failing CreateProcess (for example trying to start an
	// ARM64 fxc.exe from an x64 build) raises a modal "计算机类型不匹配" hard-error
	// dialog, which blocks unattended shader builds. Suppress it and let the
	// exception be reported through the console like every other error.
	private const uint SEM_FAILCRITICALERRORS = 0x0001;
	private const uint SEM_NOGPFAULTERRORBOX = 0x0002;

	[DllImport("kernel32.dll")]
	private static extern uint SetErrorMode(uint uMode);

	private static void Main(string[] args)
	{
		try
		{
			if (Environment.OSVersion.Platform == PlatformID.Win32NT)
			{
				SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
			}
		}
		catch (Exception)
		{
		}
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
