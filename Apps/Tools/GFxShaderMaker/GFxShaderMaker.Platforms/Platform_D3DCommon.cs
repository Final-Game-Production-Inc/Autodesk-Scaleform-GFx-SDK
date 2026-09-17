using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text.RegularExpressions;
using Microsoft.Win32;

namespace GFxShaderMaker.Platforms;

public abstract class Platform_D3DCommon : ShaderPlatformBinaryShaders
{
	public override string PlatformBinarySourceFilename
	{
		get
		{
			string option = CommandLineParser.GetOption(CommandLineParser.Options.OutputDirectory);
			return Path.Combine(option, base.PlatformName + "_" + CommandLineParser.GetOption(CommandLineParser.Options.Config) + "_ShaderBinary.cpp");
		}
	}

	public override string PlatformSourceSourceFilename
	{
		get
		{
			string option = CommandLineParser.GetOption(CommandLineParser.Options.OutputDirectory);
			return Path.Combine(option, PlatformBase + "_" + base.PlatformName + CommandLineParser.GetOption(CommandLineParser.Options.Config) + "_ShaderSource.cpp");
		}
	}

	protected abstract string D3DSDKEnvironmentVariable { get; }

	protected virtual bool PreferWindowsKitsFXC => true;

	protected virtual string D3DFXCExtraOptions
	{
		get
		{
			string option = CommandLineParser.GetOption(CommandLineParser.Options.Config);
			switch (option)
			{
			case "Debug":
			case "DebugOpt":
				return "/Zi";
			case "Release":
				return "/Zi /O3";
			case "Shipping":
				return "/O3 /Qstrip_reflect /Qstrip_debug";
			default:
				throw new Exception("Unsupported configuration type: " + option);
			}
		}
	}

	static Platform_D3DCommon()
	{
	}

	protected override string GetBinaryShaderDeclaration(ShaderPipeline pipeline)
	{
		return "const BYTE*    pBinary;";
	}

	protected override string GetBinaryShaderReference(ShaderPipeline pipeline, string id)
	{
		return $"pBinary_{id},";
	}

	protected override string GetBinaryShaderExtern(ShaderPipeline pipeline, string id)
	{
		return $"extern const BYTE pBinary_{id}[];";
	}

	protected override void WriteBinarySource(StreamWriter sourceFile)
	{
		if (base.PlatformName != "X360")
		{
			sourceFile.WriteLine("typedef unsigned char BYTE;\n");
		}
		else
		{
			sourceFile.WriteLine("#include <xtl.h> // DWORD\n\n");
		}
		base.WriteBinarySource(sourceFile);
	}

	public override void CreateShaderOutput()
	{
		string text = LocateShaderCompiler();
		if (string.IsNullOrEmpty(text))
		{
			throw new Exception("Could not locate fxc.exe - Windows Kits/DirectX SDK/XDK installation not found.");
		}
		if (CommandLineParser.GetOption<int>(CommandLineParser.Options.Verbosity) > 1)
		{
			Console.WriteLine("Using fxc.exe: {0}\n", text);
		}
		List<CompileThreadData> list = new List<CompileThreadData>();
		foreach (ShaderVersion requestedShaderVersion in RequestedShaderVersions)
		{
			foreach (ShaderLinkedSource value in requestedShaderVersion.LinkedSourceDuplicates.Values)
			{
				CompileThreadData item = new CompileThreadData(this, requestedShaderVersion, value, text);
				list.Add(item);
			}
		}
		CompileShadersThreaded(list);
		CreateBinarySource();
	}

	protected virtual string LocateShaderCompiler()
	{
		string text = "";
		string text2 = Environment.GetEnvironmentVariable(D3DSDKEnvironmentVariable);
		try
		{
			if (PreferWindowsKitsFXC)
			{
				List<string> list = new List<string>();
				RegistryKey localMachine = Registry.LocalMachine;
				localMachine = localMachine.OpenSubKey("SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots");
				string[] valueNames = localMachine.GetValueNames();
				foreach (string text3 in valueNames)
				{
					if (Regex.IsMatch(text3, "KitsRoot"))
					{
						list.Add(localMachine.GetValue(text3) as string);
					}
				}
				list.Sort(delegate(string s0, string s1)
				{
					s0 = s0.TrimEnd(Path.DirectorySeparatorChar);
					s1 = s1.TrimEnd(Path.DirectorySeparatorChar);
					string[] array = s0.Split(Path.DirectorySeparatorChar);
					string[] array2 = s1.Split(Path.DirectorySeparatorChar);
					return Convert.ToDouble(array2[array2.Length - 1]).CompareTo(Convert.ToDouble(array[array.Length - 1]));
				});
				foreach (string item in list)
				{
					if (CommandLineParser.GetOption<int>(CommandLineParser.Options.Verbosity) > 1)
					{
						Console.WriteLine("Attempting to use '{1}' as Windows Kits root.", item);
					}
					if (string.IsNullOrEmpty(item))
					{
						throw new Exception("Could not locate Windows Kits root directory. Check 'HKLM\\SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots' for proper installation.");
					}
					string text4 = Path.Combine(item, "Bin\\x64");
					IEnumerable<string> files = Directory.GetFiles(text4, "fxc.exe", SearchOption.AllDirectories);
					if (files.Count() > 0)
					{
						text = files.First();
						text2 = null;
						break;
					}
					if (string.IsNullOrEmpty(text))
					{
						if (CommandLineParser.GetOption<int>(CommandLineParser.Options.Verbosity) != 0)
						{
							Console.WriteLine("fxc.exe did not exist in expected location ({0}). Attempting to locate alternate.", text4);
						}
						files = Directory.GetFiles(item, "fxc.exe", SearchOption.AllDirectories);
						if (files.Count() > 0)
						{
							text = files.First();
							text2 = null;
							break;
						}
					}
					Console.WriteLine("Warning: no fxc.exe found within {0}. Trying next Windows Kit.", item);
				}
			}
		}
		catch (Exception)
		{
		}
		if (!string.IsNullOrEmpty(text2))
		{
			IEnumerable<string> files2 = Directory.GetFiles(text2, "fxc.exe", SearchOption.AllDirectories);
			if (files2.Count() > 0)
			{
				text = files2.First();
			}
		}
		return text;
	}

	protected override void CompileSingleShaderImpl(CompileThreadData ctdata)
	{
		if (ctdata != null)
		{
			Platform_D3DCommon platform_D3DCommon = ctdata.This as Platform_D3DCommon;
			ShaderLinkedSource source = ctdata.Source;
			string exe = ctdata.Exe;
			ShaderVersion_D3DCommon shaderVersion_D3DCommon = ctdata.SVersion as ShaderVersion_D3DCommon;
			string text = Path.Combine(ctdata.SVersion.SourceDirectory, shaderVersion_D3DCommon.GetShaderFilename(source));
			if (!File.Exists(text))
			{
				throw new Exception("Expected to find " + text + " shader source, but it did not exist.");
			}
			string shaderProfile = shaderVersion_D3DCommon.GetShaderProfile(source.Pipeline);
			string text2 = "/nologo /E main /T " + shaderProfile + " /Fh \"" + text + "\".h /Vn pBinary_" + shaderVersion_D3DCommon.ID + "_" + source.ID + " " + platform_D3DCommon.D3DFXCExtraOptions + "  " + shaderVersion_D3DCommon.GetD3DFXCExtraOptions(exe, source) + " \"" + text + "\"";
			ctdata.ExitCode = launchProcess(exe, text2, out ctdata.StdOutput, out ctdata.StdError);
			ctdata.ShaderFilename = text;
			ctdata.CommandLine = exe + " " + text2;
		}
	}
}
