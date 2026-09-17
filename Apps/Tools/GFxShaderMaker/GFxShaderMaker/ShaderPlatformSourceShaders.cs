using System;
using System.IO;

namespace GFxShaderMaker;

public abstract class ShaderPlatformSourceShaders : ShaderPlatform
{
	public override void CreateShaderOutput()
	{
		File.Delete(PlatformSourceSourceFilename);
		StreamWriter streamWriter = File.CreateText(PlatformSourceSourceFilename);
		streamWriter.Write(CopyrightNotice(PlatformSourceSourceFilename) + "\n");
		streamWriter.WriteLine("namespace Scaleform { namespace Render { namespace " + PlatformBase + " {\n\n");
		foreach (ShaderVersion requestedShaderVersion in RequestedShaderVersions)
		{
			foreach (ShaderLinkedSource value in requestedShaderVersion.LinkedSourceDuplicates.Values)
			{
				string path = Path.Combine(PlatformObjDirectory, requestedShaderVersion.GetShaderFilename(value));
				StreamReader streamReader = File.OpenText(path);
				string text = streamReader.ReadToEnd();
				streamWriter.Write("extern const char* pSource_" + requestedShaderVersion.ID + "_" + value.ID + ";\n");
				streamWriter.Write("const char* pSource_" + requestedShaderVersion.ID + "_" + value.ID + " = ");
				string[] array = text.Split("\n".ToCharArray(), StringSplitOptions.RemoveEmptyEntries);
				foreach (string text2 in array)
				{
					string text3 = text2.Trim();
					if (text3.Length != 0)
					{
						streamWriter.Write("\n\"" + text3 + "\\n\"");
					}
				}
				streamWriter.Write(";\n\n");
				streamReader.Close();
			}
		}
		streamWriter.WriteLine("}}}; // Scaleform::Render::" + PlatformBase + "\n\n");
		streamWriter.Close();
		if (CommandLineParser.GetOption<int>(CommandLineParser.Options.Verbosity) != 0)
		{
			Console.WriteLine("Wrote: " + PlatformSourceSourceFilename);
		}
	}

	protected override void writeHeaderPipelineShaderDataMembers(IndentStreamWriter headerFile, ShaderPipeline pipeline)
	{
		headerFile.Write("const char*     pSource;\n");
	}

	protected override void writeSourcePipelineShaderGlobals(IndentStreamWriter sourceFile, ShaderVersion ver, ShaderPipeline pipeline, string srcID)
	{
		sourceFile.Write("extern const char* pSource_" + ver.ID + "_" + srcID + ";\n");
	}

	protected override void writeSourcePipelineShaderData(IndentStreamWriter sourceFile, ShaderVersion ver, ShaderPipeline pipeline, ShaderLinkedSource src)
	{
		sourceFile.Write("/* pSource */       pSource_" + ver.ID + "_" + src.SourceCodeDuplicateID + ",\n");
	}
}
