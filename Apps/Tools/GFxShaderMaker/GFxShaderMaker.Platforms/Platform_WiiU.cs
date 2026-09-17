using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace GFxShaderMaker.Platforms;

[Platform("WiiU", "Nintendo WiiU GLSL")]
public class Platform_WiiU : ShaderPlatformBinaryShaders
{
	private List<ShaderVersion> ShaderVersions = new List<ShaderVersion>();

	public override List<ShaderVersion> RequestedShaderVersions => ShaderVersions;

	public string SDKEnvironmentVariable => "CAFE_ROOT";

	public Platform_WiiU()
	{
		ShaderVersions.Add(new WiiU_Version(this));
	}

	protected override string GetBinaryShaderDeclaration(ShaderPipeline pipeline)
	{
		return pipeline.Type switch
		{
			ShaderPipeline.PipelineType.Vertex => "const GX2VertexShader*    pBinary;\n", 
			_ => "const GX2PixelShader*     pBinary;\n", 
		};
	}

	protected override string GetBinaryShaderReference(ShaderPipeline pipeline, string id)
	{
		return pipeline.Type switch
		{
			ShaderPipeline.PipelineType.Vertex => $"&{id}_VS,", 
			_ => $"&{id}_PS,", 
		};
	}

	protected override string GetBinaryShaderExtern(ShaderPipeline pipeline, string id)
	{
		return pipeline.Type switch
		{
			ShaderPipeline.PipelineType.Vertex => $"extern GX2VertexShader    {id}_VS;", 
			_ => $"extern GX2PixelShader     {id}_PS;", 
		};
	}

	protected override void WriteBinarySource(StreamWriter sourceFile)
	{
		sourceFile.WriteLine("\r\n#include <cafe/gx2.h>\r\n\r\ntypedef struct _GFDLoopVar\r\n{\r\n    u32 reg[GX2_NUM_LOOP_VAR_U32_WORDS];\r\n} GFDLoopVar;\r\n\r\n\r\n");
		base.WriteBinarySource(sourceFile);
	}

	public override void CreateShaderOutput()
	{
		string f = "";
		string text = LocateCafeSDK();
		if (text == null)
		{
			throw new Exception("Could not locate Cafe SDK (must set environment variable CAFE_ROOT).");
		}
		if (!string.IsNullOrEmpty(text))
		{
			Path.Combine(text, "\\system\\bin\\win32");
			IEnumerable<string> files = Directory.GetFiles(text, "gshCompile.exe", SearchOption.AllDirectories);
			if (files.Count() > 0)
			{
				f = files.First();
			}
		}
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
		CreateBinarySource();
	}

	private string LocateCafeSDK()
	{
		string environmentVariable = Environment.GetEnvironmentVariable(SDKEnvironmentVariable + "_DOS");
		if (!string.IsNullOrEmpty(environmentVariable) && Directory.Exists(environmentVariable))
		{
			return environmentVariable;
		}
		environmentVariable = Environment.GetEnvironmentVariable(SDKEnvironmentVariable);
		if (!string.IsNullOrEmpty(environmentVariable) && Directory.Exists(environmentVariable))
		{
			return environmentVariable;
		}
		environmentVariable = "C:\\CAFE_SDK";
		if (!string.IsNullOrEmpty(environmentVariable) && Directory.Exists(environmentVariable))
		{
			return environmentVariable;
		}
		return null;
	}

	protected override void CompileSingleShaderImpl(CompileThreadData ctdata)
	{
		if (ctdata != null)
		{
			Platform_WiiU platform_WiiU = ctdata.This as Platform_WiiU;
			WiiU_Version wiiU_Version = ctdata.SVersion as WiiU_Version;
			ShaderLinkedSource source = ctdata.Source;
			string exe = ctdata.Exe;
			string text = Path.Combine(ctdata.SVersion.SourceDirectory, wiiU_Version.GetShaderFilename(source));
			if (!File.Exists(text))
			{
				throw new Exception("Expected to find " + text + " shader source, but it did not exist.");
			}
			string shaderProfile = platform_WiiU.GetShaderProfile(source.Pipeline);
			string shaderOutputFilename = wiiU_Version.GetShaderOutputFilename(source);
			File.Delete(shaderOutputFilename);
			string text2 = "-" + shaderProfile + " \"" + text + "\" -oh \"" + shaderOutputFilename + "\"";
			ctdata.ExitCode = launchProcess(exe, text2, out ctdata.StdOutput, out ctdata.StdError);
			ctdata.ShaderFilename = text;
			ctdata.CommandLine = exe + " " + text2;
		}
	}

	private string GetShaderProfile(ShaderPipeline pipeline)
	{
		return pipeline.Type switch
		{
			ShaderPipeline.PipelineType.Fragment => "p", 
			_ => "v", 
		};
	}

	protected override void writeHeaderPreamble(IndentStreamWriter headerFile)
	{
		headerFile.Write("#include <cafe/gx2.h> // GX2PixelShader/GX2VertexShader\n\n");
		base.writeHeaderPreamble(headerFile);
	}
}
