using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace GFxShaderMaker.Platforms;

[Platform("Metal", "iOS 8.0+ Metal")]
public class Platform_Metal : ShaderPlatformDataFileShaders
{
	private List<ShaderVersion> ShaderVersions = new List<ShaderVersion>();

	public override string PlatformSourceExtension => ".mm";

	public override List<ShaderVersion> RequestedShaderVersions => ShaderVersions;

	protected string MetalCompilationOptions
	{
		get
		{
			string option = CommandLineParser.GetOption(CommandLineParser.Options.Config);
			string text = "-arch air64 -emit-llvm -c -std=ios-metal1.0 ";
			switch (option)
			{
			case "Debug":
			case "DebugOpt":
				text += "-gline-tables-only ";
				break;
			}
			return text;
		}
	}

	public Platform_Metal()
	{
		ShaderVersions.Add(new ShaderVersion_Metal(this));
	}

	public override void CreateShaderOutput()
	{
		int num = 0;
		string executable = "/usr/bin/xcode-select";
		if (launchProcess(executable, "-p", out var stdout, out var _) != 0)
		{
			throw new Exception("Could not run 'xcode-select' executable. Ensure Xcode is properly installed.");
		}
		string text = stdout.Trim();
		string path = Path.Combine(text, "Platforms/iPhoneOS.platform");
		string text2 = "";
		string text3 = "";
		string text4 = "";
		if (!string.IsNullOrEmpty(text))
		{
			IEnumerable<string> files = Directory.GetFiles(path, "metal", SearchOption.AllDirectories);
			if (files.Count() <= 0)
			{
				throw new Exception("Could not locate 'metal' exectuable. Ensure Xcode 6+ is installed properly, and set using xcode-select utility.");
			}
			text2 = files.First();
			text3 = Path.Combine(Path.GetDirectoryName(text2), "metal-ar");
			text4 = Path.Combine(Path.GetDirectoryName(text2), "metallib");
			if (CommandLineParser.GetOption<int>(CommandLineParser.Options.Verbosity) > 1)
			{
				Console.WriteLine("metal    path = " + text2);
				Console.WriteLine("metal-ar path = " + text3);
				Console.WriteLine("metallib path = " + text4);
			}
		}
		if (!Directory.Exists(PlatformObjDirectory))
		{
			Directory.CreateDirectory(PlatformObjDirectory);
		}
		List<CompileThreadData> list = new List<CompileThreadData>();
		foreach (ShaderVersion requestedShaderVersion in RequestedShaderVersions)
		{
			foreach (ShaderLinkedSource value in requestedShaderVersion.LinkedSourceDuplicates.Values)
			{
				CompileThreadData item = new CompileThreadData(this, requestedShaderVersion, value, text2);
				list.Add(item);
			}
		}
		CompileShadersThreaded(list);
		foreach (ShaderVersion requestedShaderVersion2 in RequestedShaderVersions)
		{
			string currentDirectory = Environment.CurrentDirectory;
			string text5 = Path.Combine(PlatformObjDirectory, requestedShaderVersion2.ID + ".metalar");
			File.Delete(text5);
			string text6 = "rcv " + text5;
			Environment.CurrentDirectory = PlatformObjDirectory;
			string stdout2;
			string stderr2;
			foreach (ShaderLinkedSource value2 in requestedShaderVersion2.LinkedSourceDuplicates.Values)
			{
				string text7 = requestedShaderVersion2.ID + "_" + value2.ID + ".air";
				text6 = "rcv " + text5 + " " + text7;
				if (launchProcess(text3, text6, out stdout2, out stderr2) != 0)
				{
					Console.WriteLine("Error creating " + text5 + ":\n");
					Console.WriteLine(stderr2);
					Console.WriteLine(stdout2);
					throw new Exception("Library creation failed.");
				}
			}
			Environment.CurrentDirectory = currentDirectory;
			if (!Directory.Exists(PlatformLibDirectory))
			{
				Directory.CreateDirectory(PlatformLibDirectory);
			}
			string option = CommandLineParser.GetOption(CommandLineParser.Options.Config);
			string text8 = Path.Combine(PlatformLibDirectory, requestedShaderVersion2.ID + "_" + option + ".metallib");
			File.Delete(text8);
			if (launchProcess(text4, "-o " + text8 + " " + text5, out stdout2, out stderr2) != 0)
			{
				Console.WriteLine("Error creating " + text8 + ":");
				Console.WriteLine(stderr2);
				Console.WriteLine(stdout2);
				throw new Exception("Library creation failed.");
			}
			Environment.CurrentDirectory = currentDirectory;
		}
	}

	protected override void CompileSingleShaderImpl(CompileThreadData ctdata)
	{
		if (ctdata != null)
		{
			Platform_Metal platform_Metal = ctdata.This as Platform_Metal;
			ShaderVersion sVersion = ctdata.SVersion;
			ShaderLinkedSource source = ctdata.Source;
			string exe = ctdata.Exe;
			string text = Path.Combine(ctdata.SVersion.SourceDirectory, sVersion.GetShaderFilename(source));
			if (!File.Exists(text))
			{
				throw new Exception("Expected to find " + text + " shader source, but it did not exist.");
			}
			string path = sVersion.ID + "_" + source.ID + ".air";
			string text2 = Path.Combine(platform_Metal.PlatformObjDirectory, path);
			if (!Directory.Exists(Path.GetDirectoryName(Path.GetDirectoryName(platform_Metal.PlatformObjDirectory))))
			{
				Directory.CreateDirectory(Path.GetDirectoryName(platform_Metal.PlatformObjDirectory));
			}
			string text3 = MetalCompilationOptions + " -o \"" + text2 + "\" \"" + text + "\"";
			ctdata.ExitCode = launchProcess(exe, text3, out ctdata.StdOutput, out ctdata.StdError);
			ctdata.ShaderFilename = text;
			ctdata.CommandLine = exe + " " + text3;
		}
	}

	public override string GeneratePipelineHeaderExtras(ShaderPipeline pipeline)
	{
		string text = base.GeneratePipelineHeaderExtras(pipeline);
		return text + "const char* ShaderName;\n";
	}

	public override string GeneratePipelineSourceExtras(ShaderVersion ver, ShaderPipeline pipeline, ShaderLinkedSource src)
	{
		string text = base.GeneratePipelineSourceExtras(ver, pipeline, src);
		return text + "/* ShaderName */    \"" + src.SourceCodeDuplicateID + "\"\n";
	}
}
