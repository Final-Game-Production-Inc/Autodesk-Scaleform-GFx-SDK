using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;

namespace GFxShaderMaker.Platforms;

[Platform("PS3", "Sony PS3 Cg")]
public class Platform_PS3 : ShaderPlatformBinaryShaders
{
	private List<ShaderVersion> ShaderVersions = new List<ShaderVersion>();

	public override List<ShaderVersion> RequestedShaderVersions => ShaderVersions;

	public override string PlatformCompiler => "snc";

	public string CGCExtraOptions
	{
		get
		{
			string option = CommandLineParser.GetOption(CommandLineParser.Options.Config);
			if (option.StartsWith("Debug"))
			{
				return "--debug ";
			}
			if (option.StartsWith("Release") || option.StartsWith("Shipping"))
			{
				return "--O3 --fastmath --fastprecision";
			}
			return "--debug --fastmath --fastprecision ";
		}
	}

	public string CELLSDKEnvironmentVariable => "SCE_PS3_ROOT";

	public Platform_PS3()
	{
		ShaderVersions.Add(new ShaderVersion_PS3(this));
	}

	protected override string GetBinaryShaderDeclaration(ShaderPipeline pipeline)
	{
		return "const int*     pBinary;";
	}

	protected override string GetBinaryShaderReference(ShaderPipeline pipeline, string id)
	{
		return $"_binary_{id}_po_start,";
	}

	protected override string GetBinaryShaderExtern(ShaderPipeline pipeline, string id)
	{
		return $"extern \"C\" const int _binary_{id}_po_start[];";
	}

	protected string GetShaderProfile(ShaderPipeline pipeline)
	{
		return pipeline.Type switch
		{
			ShaderPipeline.PipelineType.Fragment => "sce_fp_rsx", 
			ShaderPipeline.PipelineType.Vertex => "sce_vp_rsx", 
			_ => null, 
		};
	}

	public override void CreateShaderOutput()
	{
		string f = "";
		string fileName = "";
		string text = "";
		string environmentVariable = Environment.GetEnvironmentVariable(CELLSDKEnvironmentVariable);
		if (!string.IsNullOrEmpty(environmentVariable))
		{
			IEnumerable<string> files = Directory.GetFiles(environmentVariable, "sce-cgc.exe", SearchOption.AllDirectories);
			if (files.Count() <= 0)
			{
				throw new Exception("Could not locate sce-cgc.exe.");
			}
			f = files.First();
			files = Directory.GetFiles(environmentVariable, "ps3snarl.exe", SearchOption.AllDirectories);
			if (files.Count() <= 0)
			{
				throw new Exception("Could not locate ps3snarl.exe.");
			}
			fileName = files.First();
			files = Directory.GetFiles(environmentVariable, "ppu-lv2-objcopy.exe", SearchOption.AllDirectories);
			if (files.Count() <= 0)
			{
				throw new Exception("Could not locate ppu-lv2-objcopy.exe.");
			}
			text = files.First();
		}
		if (!Directory.Exists(PlatformObjDirectory))
		{
			Directory.CreateDirectory(PlatformObjDirectory);
		}
		string text2 = "rc \"" + PlatformBinaryLibrary + "\"";
		List<CompileThreadData> list = new List<CompileThreadData>();
		foreach (ShaderVersion requestedShaderVersion in RequestedShaderVersions)
		{
			foreach (ShaderLinkedSource value3 in requestedShaderVersion.LinkedSourceDuplicates.Values)
			{
				CompileThreadData item = new CompileThreadData(this, requestedShaderVersion, value3, f);
				list.Add(item);
			}
		}
		CompileShadersThreaded(list);
		foreach (ShaderVersion requestedShaderVersion2 in RequestedShaderVersions)
		{
			string currentDirectory = Environment.CurrentDirectory;
			foreach (ShaderLinkedSource value4 in requestedShaderVersion2.LinkedSourceDuplicates.Values)
			{
				Environment.CurrentDirectory = currentDirectory;
				string text3 = requestedShaderVersion2.ID + "_" + value4.ID + ".o";
				string text4 = Path.Combine(PlatformObjDirectory, text3);
				Environment.CurrentDirectory = PlatformObjDirectory;
				string text5 = "-I binary -O elf64-powerpc-celloslv2 -B powerpc " + requestedShaderVersion2.ID + "_" + value4.ID + ".po " + requestedShaderVersion2.ID + "_" + value4.ID + ".o";
				text2 = text2 + " \"" + text3 + "\"";
				ProcessStartInfo processStartInfo = new ProcessStartInfo(text, text5);
				processStartInfo.ErrorDialog = false;
				processStartInfo.CreateNoWindow = true;
				processStartInfo.UseShellExecute = false;
				processStartInfo.RedirectStandardError = true;
				processStartInfo.RedirectStandardOutput = true;
				Process process = Process.Start(processStartInfo);
				string value = process.StandardError.ReadToEnd();
				process.WaitForExit();
				if (process.ExitCode != 0)
				{
					Console.WriteLine("Error creating " + text4 + ":");
					Console.WriteLine(text + " " + text5);
					Console.WriteLine(value);
					throw new Exception("Error creating " + text4);
				}
			}
			Environment.CurrentDirectory = currentDirectory;
		}
		if (!Directory.Exists(Path.GetDirectoryName(PlatformBinaryLibrary)))
		{
			Directory.CreateDirectory(Path.GetDirectoryName(PlatformBinaryLibrary));
		}
		File.Delete(PlatformBinaryLibrary);
		ProcessStartInfo processStartInfo2 = new ProcessStartInfo(fileName, text2);
		processStartInfo2.ErrorDialog = false;
		processStartInfo2.CreateNoWindow = true;
		processStartInfo2.UseShellExecute = false;
		processStartInfo2.RedirectStandardError = true;
		processStartInfo2.RedirectStandardOutput = true;
		processStartInfo2.WorkingDirectory = PlatformObjDirectory;
		Process process2 = Process.Start(processStartInfo2);
		string value2 = process2.StandardError.ReadToEnd();
		process2.WaitForExit();
		if (process2.ExitCode != 0)
		{
			Console.WriteLine("Error creating " + PlatformBinaryLibrary + ":");
			Console.WriteLine(value2);
			throw new Exception("Error creating " + PlatformBinaryLibrary);
		}
	}

	protected override void CompileSingleShaderImpl(CompileThreadData ctdata)
	{
		if (ctdata != null)
		{
			Platform_PS3 platform_PS = ctdata.This as Platform_PS3;
			ShaderVersion sVersion = ctdata.SVersion;
			ShaderLinkedSource source = ctdata.Source;
			string exe = ctdata.Exe;
			string text = Path.Combine(ctdata.SVersion.SourceDirectory, sVersion.GetShaderFilename(source));
			if (!File.Exists(text))
			{
				throw new Exception("Expected to find " + text + " shader source, but it did not exist.");
			}
			string path = sVersion.ID + "_" + source.ID + ".po";
			string shaderProfile = platform_PS.GetShaderProfile(source.Pipeline);
			string text2 = Path.Combine(PlatformObjDirectory, path);
			string file = text2;
			string file2 = text;
			Uri uri = GreatestCommonPath(ref file2, ref file);
			string text3 = "--entry main --profile " + shaderProfile + " --output \"" + file + "\" " + platform_PS.CGCExtraOptions + " \"" + file2 + "\"";
			ctdata.ExitCode = launchProcess(exe, text3, uri.LocalPath, out ctdata.StdOutput, out ctdata.StdError);
			if (ctdata.ExitCode == 0 && !File.Exists(text2))
			{
				ctdata.ExitCode = -255;
			}
			ctdata.ShaderFilename = text;
			ctdata.CommandLine = exe + " " + text3;
		}
	}
}
