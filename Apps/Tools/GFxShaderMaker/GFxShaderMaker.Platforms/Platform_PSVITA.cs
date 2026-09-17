using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace GFxShaderMaker.Platforms;

[Platform("PSVita", "Sony PS VITA Cg")]
public class Platform_PSVITA : ShaderPlatformBinaryShaders
{
	private List<ShaderVersion> ShaderVersions = new List<ShaderVersion>();

	public override List<ShaderVersion> RequestedShaderVersions => ShaderVersions;

	public string CGCExtraOptions
	{
		get
		{
			string option = CommandLineParser.GetOption(CommandLineParser.Options.Config);
			if (option.StartsWith("Debug"))
			{
				return "-cache ";
			}
			return "-cache -fastprecision ";
		}
	}

	public string PSP2SDKEnvironmentVariable => "SCE_PSP2_SDK_DIR";

	public Platform_PSVITA()
	{
		ShaderVersions.Add(new ShaderVersion_PSVITA(this));
	}

	protected override string GetBinaryShaderDeclaration(ShaderPipeline pipeline)
	{
		return "const SceGxmProgram*     pBinary;";
	}

	protected override string GetBinaryShaderReference(ShaderPipeline pipeline, string id)
	{
		return $"_binary_{id}_gxp,";
	}

	protected override string GetBinaryShaderExtern(ShaderPipeline pipeline, string id)
	{
		return $"extern \"C\" const SceGxmProgram _binary_{id}_gxp[];";
	}

	protected string GetShaderProfile(ShaderPipeline pipeline)
	{
		return pipeline.Type switch
		{
			ShaderPipeline.PipelineType.Fragment => "sce_fp_psp2", 
			ShaderPipeline.PipelineType.Vertex => "sce_vp_psp2", 
			_ => null, 
		};
	}

	public override void CreateShaderOutput()
	{
		string f = "";
		string executable = "";
		string text = "";
		string environmentVariable = Environment.GetEnvironmentVariable(PSP2SDKEnvironmentVariable);
		if (!string.IsNullOrEmpty(environmentVariable))
		{
			IEnumerable<string> files = Directory.GetFiles(environmentVariable, "psp2cgc.exe", SearchOption.AllDirectories);
			if (files.Count() <= 0)
			{
				throw new Exception("Could not locate psp2cgc.exe.");
			}
			f = files.First();
			files = Directory.GetFiles(environmentVariable, "psp2snarl.exe", SearchOption.AllDirectories);
			if (files.Count() <= 0)
			{
				throw new Exception("Could not locate psp2snarl.exe.");
			}
			executable = files.First();
			files = Directory.GetFiles(environmentVariable, "psp2bin.exe", SearchOption.AllDirectories);
			if (files.Count() <= 0)
			{
				throw new Exception("Could not locate psp2bin.exe.");
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
			foreach (ShaderLinkedSource value in requestedShaderVersion.LinkedSourceDuplicates.Values)
			{
				CompileThreadData item = new CompileThreadData(this, requestedShaderVersion, value, f);
				list.Add(item);
			}
		}
		CompileShadersThreaded(list);
		string stdout;
		string stderr;
		foreach (ShaderVersion requestedShaderVersion2 in RequestedShaderVersions)
		{
			string currentDirectory = Environment.CurrentDirectory;
			foreach (ShaderLinkedSource value2 in requestedShaderVersion2.LinkedSourceDuplicates.Values)
			{
				Environment.CurrentDirectory = currentDirectory;
				string text3 = requestedShaderVersion2.ID + "_" + value2.ID;
				string text4 = text3 + ".o";
				string text5 = Path.Combine(PlatformObjDirectory, text4);
				Environment.CurrentDirectory = PlatformObjDirectory;
				string text6 = "-i " + text3 + ".gxp -o " + text3 + ".o -b2e PSP2,_binary_" + text3 + "_gxp,_binary_" + text3 + "_gxp_size";
				text2 = text2 + " \"" + text4 + "\"";
				if (launchProcess(text, text6, out stdout, out stderr) != 0)
				{
					Console.WriteLine("Error creating " + text5 + ":");
					Console.WriteLine(text + " " + text6);
					Console.WriteLine(stderr);
					throw new Exception("Error creating " + text5);
				}
			}
			Environment.CurrentDirectory = currentDirectory;
		}
		if (!Directory.Exists(Path.GetDirectoryName(PlatformBinaryLibrary)))
		{
			Directory.CreateDirectory(Path.GetDirectoryName(PlatformBinaryLibrary));
		}
		File.Delete(PlatformBinaryLibrary);
		if (launchProcess(executable, text2, PlatformObjDirectory, out stdout, out stderr) != 0)
		{
			Console.WriteLine("Error creating " + PlatformBinaryLibrary + ":");
			Console.WriteLine(stderr);
			throw new Exception("Error creating " + PlatformBinaryLibrary);
		}
	}

	protected override void CompileSingleShaderImpl(CompileThreadData ctdata)
	{
		if (ctdata != null)
		{
			Platform_PSVITA platform_PSVITA = ctdata.This as Platform_PSVITA;
			ShaderVersion sVersion = ctdata.SVersion;
			ShaderLinkedSource source = ctdata.Source;
			string exe = ctdata.Exe;
			string text = Path.Combine(ctdata.SVersion.SourceDirectory, sVersion.GetShaderFilename(source));
			if (!File.Exists(text))
			{
				throw new Exception("Expected to find " + text + " shader source, but it did not exist.");
			}
			string file = Path.Combine(PlatformObjDirectory, sVersion.ID + "_" + source.ID) + ".gxp";
			string path = file;
			string file2 = text;
			Uri uri = GreatestCommonPath(ref file2, ref file);
			string shaderProfile = platform_PSVITA.GetShaderProfile(source.Pipeline);
			string text2 = "-profile " + shaderProfile + " -o \"" + file + "\" " + platform_PSVITA.CGCExtraOptions + " \"" + file2 + "\"";
			ctdata.ExitCode = launchProcess(exe, text2, uri.LocalPath, out ctdata.StdOutput, out ctdata.StdError);
			if (ctdata.ExitCode == 0 && !File.Exists(path))
			{
				ctdata.ExitCode = -255;
			}
			ctdata.ShaderFilename = text;
			ctdata.CommandLine = exe + " " + text2;
		}
	}

	protected override void writeHeaderPreamble(IndentStreamWriter headerFile)
	{
		headerFile.Write("#include <gxm/program.h> // SceGxmProgram\n\n");
		base.writeHeaderPreamble(headerFile);
	}
}
