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
			Console.WriteLine("Using shader compiler: {0}\n", text);
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
		int verbosity = CommandLineParser.GetOption<int>(CommandLineParser.Options.Verbosity);
		string overridePath = Environment.GetEnvironmentVariable(ShaderCompilerLocator.FxcOverrideEnvironmentVariable);
		if (!string.IsNullOrEmpty(overridePath))
		{
			if (File.Exists(overridePath) && ShaderCompilerLocator.CanExecute(ShaderCompilerLocator.ReadMachineType(overridePath)))
			{
				if (verbosity > 1)
				{
					Console.WriteLine("Using {0} from ${1}.", overridePath, ShaderCompilerLocator.FxcOverrideEnvironmentVariable);
				}
				return overridePath;
			}
			if (verbosity != 0)
			{
				Console.WriteLine("Warning: ${0} = '{1}' is missing or cannot run on this machine. Ignoring it.", ShaderCompilerLocator.FxcOverrideEnvironmentVariable, overridePath);
			}
		}

		string compiler = null;
		if (PreferWindowsKitsFXC)
		{
			// <KitsRoot>\Bin\<version>\<arch>\fxc.exe, newest SDK / native architecture first.
			compiler = ShaderCompilerLocator.FindInWindowsKits("fxc.exe", verbosity);
		}
		if (compiler == null)
		{
			// DirectX SDK (DXSDK_DIR), XDK (XEDK) and DurangoXDK still ship usable fxc.exe.
			compiler = ShaderCompilerLocator.FindInRoot(Environment.GetEnvironmentVariable(D3DSDKEnvironmentVariable), "fxc.exe", verbosity);
		}
		if (compiler == null && !PreferWindowsKitsFXC)
		{
			compiler = ShaderCompilerLocator.FindInWindowsKits("fxc.exe", verbosity);
		}
		return compiler;
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
			bool useDxc = shaderProfile.Contains("_6_");
			string text2;
			if (useDxc)
			{
				// D3D12 pipeline states require DXIL (SM 6.x); fxc can only emit
				// DXBC (SM 5.x) and D3D12 rejects it at PSO creation time.
				text2 = "-nologo -E main -T " + shaderProfile + " -Fh \"" + text + "\".h -Vn pBinary_" + shaderVersion_D3DCommon.ID + "_" + source.ID + " " + platform_D3DCommon.D3DFXCExtraOptions + "  " + shaderVersion_D3DCommon.GetD3DFXCExtraOptions(exe, source) + " \"" + text + "\"";
			}
			else
			{
				text2 = "/nologo /E main /T " + shaderProfile + " /Fh \"" + text + "\".h /Vn pBinary_" + shaderVersion_D3DCommon.ID + "_" + source.ID + " " + platform_D3DCommon.D3DFXCExtraOptions + "  " + shaderVersion_D3DCommon.GetD3DFXCExtraOptions(exe, source) + " \"" + text + "\"";
			}
			ctdata.ExitCode = launchProcess(exe, text2, out ctdata.StdOutput, out ctdata.StdError);
			ctdata.ShaderFilename = text;
			ctdata.CommandLine = exe + " " + text2;
		}
	}
}
