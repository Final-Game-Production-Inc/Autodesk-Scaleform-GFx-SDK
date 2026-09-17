using System;
using System.IO;

namespace GFxShaderMaker;

public abstract class ShaderPlatformBinaryShaders : ShaderPlatform
{
	protected virtual string GetBinaryShaderDeclaration(ShaderPipeline pipeline)
	{
		throw new Exception("This method must be overridden for platforms that use binary shaders.");
	}

	protected virtual string GetBinaryShaderReference(ShaderPipeline pipeline, string ID)
	{
		throw new Exception("This method must be overridden for platforms that use binary shaders.");
	}

	protected virtual string GetBinaryShaderExtern(ShaderPipeline pipeline, string ID)
	{
		throw new Exception("This method must be overridden for platforms that use binary shaders.");
	}

	public void CreateBinarySource()
	{
		if (!Directory.Exists(Path.GetDirectoryName(PlatformBinarySourceFilename)))
		{
			Directory.CreateDirectory(Path.GetDirectoryName(PlatformBinarySourceFilename));
		}
		File.Delete(PlatformBinarySourceFilename);
		StreamWriter streamWriter = File.CreateText(PlatformBinarySourceFilename);
		streamWriter.Write(CopyrightNotice(PlatformBinarySourceFilename) + "\n");
		WriteBinarySource(streamWriter);
		streamWriter.Close();
		if (CommandLineParser.GetOption<int>(CommandLineParser.Options.Verbosity) != 0)
		{
			Console.WriteLine("Wrote: " + PlatformBinarySourceFilename);
		}
	}

	protected virtual void WriteBinarySource(StreamWriter sourceFile)
	{
		sourceFile.WriteLine("namespace Scaleform { namespace Render { namespace " + PlatformBase + " {\n\n");
		foreach (ShaderVersion requestedShaderVersion in RequestedShaderVersions)
		{
			requestedShaderVersion.WriteBinaryShaderSource(sourceFile);
		}
		sourceFile.WriteLine("}}}; // Scaleform::Render::" + PlatformBase + "\n\n");
	}

	protected override void writeHeaderPipelineShaderDataMembers(IndentStreamWriter headerFile, ShaderPipeline pipeline)
	{
		string binaryShaderDeclaration = GetBinaryShaderDeclaration(pipeline);
		string[] array = binaryShaderDeclaration.Split("\n".ToCharArray(), StringSplitOptions.RemoveEmptyEntries);
		foreach (string text in array)
		{
			headerFile.Write(text + "\n");
		}
	}

	protected override void writeSourcePipelineShaderGlobals(IndentStreamWriter sourceFile, ShaderVersion ver, ShaderPipeline pipeline, string srcID)
	{
		string binaryShaderExtern = GetBinaryShaderExtern(pipeline, ver.ID + "_" + srcID);
		string[] array = binaryShaderExtern.Split("\n".ToCharArray(), StringSplitOptions.RemoveEmptyEntries);
		foreach (string text in array)
		{
			sourceFile.Write(text + "\n");
		}
	}

	protected override void writeSourcePipelineShaderData(IndentStreamWriter sourceFile, ShaderVersion ver, ShaderPipeline pipeline, ShaderLinkedSource src)
	{
		string binaryShaderReference = GetBinaryShaderReference(pipeline, ver.ID + "_" + src.SourceCodeDuplicateID);
		string[] array = binaryShaderReference.Split("\n".ToCharArray(), StringSplitOptions.RemoveEmptyEntries);
		foreach (string text in array)
		{
			sourceFile.Write("/* pBinary */       " + text + "\n");
		}
	}
}
