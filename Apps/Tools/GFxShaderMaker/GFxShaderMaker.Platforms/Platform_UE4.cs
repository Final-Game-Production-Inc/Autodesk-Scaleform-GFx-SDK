using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text.RegularExpressions;

namespace GFxShaderMaker.Platforms;

[Platform("UE4", "Unreal Engine 4")]
public class Platform_UE4 : ShaderPlatformSourceShaders
{
	public enum CommandLineOptions
	{
		[CommandLineOption(CmdLineOptionType.OptionType_String, "ue4path", null, "The UE4 base path to update with the compiled shaders.", null)]
		UE4Path
	}

	private List<ShaderVersion> ShaderVersions = new List<ShaderVersion>();

	public override List<ShaderVersion> RequestedShaderVersions => ShaderVersions;

	public override string PlatformBase => "RHI";

	public override string PlatformHeaderFilename => Path.Combine(CommandLineParser.GetOption(CommandLineParser.Options.OutputDirectory), "RHI_ShaderDescs.h");

	public override string PlatformSourceFilename => Path.Combine(CommandLineParser.GetOption(CommandLineParser.Options.OutputDirectory), "RHI_ShaderDescs.cpp");

	public Platform_UE4()
	{
		ShaderVersions.Add(new ShaderVersion_UE4(this));
	}

	public override void WriteShaderSources()
	{
		base.WriteShaderSources();
	}

	public override void CreateShaderOutput()
	{
	}

	protected override void writeHeaderPipelineDescFunctions(IndentStreamWriter headerFile, ShaderPipeline pipeline)
	{
		base.writeHeaderPipelineDescFunctions(headerFile, pipeline);
		string name = pipeline.Name;
		_ = pipeline.Letter;
		headerFile.Write("static " + name + "Shader* GetShader(ShaderIndex index, ShaderDesc::ShaderVersion ver = ShaderDesc::ShaderVersion_Default);\n");
	}

	protected override void writeSourcePipelineDescFunctions(IndentStreamWriter sourceFile, ShaderPipeline pipeline)
	{
		base.writeSourcePipelineDescFunctions(sourceFile, pipeline);
		string name = pipeline.Name;
		char letter = pipeline.Letter;
		sourceFile.Write(name + "Shader* " + name + "ShaderDesc::GetShader( ShaderIndex shaderIndex, ShaderDesc::ShaderVersion ver)\n");
		sourceFile.Write("{\n");
		sourceFile.Write("switch(ver)\n{\n");
		foreach (ShaderVersion requestedShaderVersion in RequestedShaderVersions)
		{
			sourceFile.Write("case ShaderDesc::ShaderVersion_" + requestedShaderVersion.ID + ":\n{\n");
			sourceFile.Write("switch( shaderIndex )\n");
			sourceFile.Write("{\n");
			foreach (List<ShaderLinkedSource> value in requestedShaderVersion.LinkedSourceUniqueDescs.Values)
			{
				ShaderLinkedSource shaderLinkedSource = value.First();
				if (shaderLinkedSource.Pipeline == pipeline)
				{
					sourceFile.Write("case " + letter + "SI_" + requestedShaderVersion.ID + "_" + shaderLinkedSource.ID + ":\n");
					sourceFile.Write("{\n");
					sourceFile.Write("TShaderMapRef< RHI::" + name + "ShaderImpl<" + letter + "SI_" + requestedShaderVersion.ID + "_" + shaderLinkedSource.SourceCodeDuplicateID + "> > NativeShader( GetGlobalShaderMap ( GMaxRHIFeatureLevel ) );\n");
					sourceFile.Write("check( NULL != *NativeShader );\n");
					sourceFile.Write("return *NativeShader;\n");
					sourceFile.Write("}\n");
				}
			}
			sourceFile.Write("default: SF_ASSERT(0); return NULL;\n");
			sourceFile.Write("}\n");
			sourceFile.Write("break;\n");
			sourceFile.Write("}\n\n");
		}
		sourceFile.Write("default: SF_ASSERT(0); return NULL;\n");
		sourceFile.Write("}\n");
		sourceFile.Write("};\n\n");
	}

	protected override uint writeSourcePipelineShaderDescs(ShaderPipeline pipeline, IndentStreamWriter sourceFile, Dictionary<int, string> uniformDefDictionary, uint shadowOffsetStart, Dictionary<int, string> batchUniformDefDictionary)
	{
		string name = pipeline.Name;
		char letter = pipeline.Letter;
		string text = ((letter == 'F') ? "Pixel" : "Vertex");
		foreach (ShaderVersion requestedShaderVersion in RequestedShaderVersions)
		{
			foreach (ShaderLinkedSource value in requestedShaderVersion.LinkedSourceDuplicates.Values)
			{
				if (value.Pipeline == pipeline)
				{
					sourceFile.Write("IMPLEMENT_SHADER_TYPE(template<>,RHI::" + name + "ShaderImpl<" + name + "ShaderDesc::");
					sourceFile.Write(letter + "SI_" + requestedShaderVersion.ID + "_" + value.ID + ">,TEXT(\"" + Path.GetFileNameWithoutExtension(requestedShaderVersion.GetShaderFilename(value)) + "\"),");
					sourceFile.Write("TEXT(\"main\"),SF_" + text + ");\n");
				}
			}
		}
		return base.writeSourcePipelineShaderDescs(pipeline, sourceFile, uniformDefDictionary, shadowOffsetStart, batchUniformDefDictionary);
	}

	protected override void writeHeaderPreamble(IndentStreamWriter headerFile)
	{
		base.writeHeaderPreamble(headerFile);
		headerFile.Write("struct VertexShader;\n");
		headerFile.Write("struct FragShader;\n");
	}

	protected override void writeSourcePreamble(IndentStreamWriter sourceFile)
	{
		sourceFile.Write("#if WITH_SCALEFORM\n");
		sourceFile.Write("#include \"ScaleformUI.h\"\n");
		sourceFile.Write("#include \"Render/" + Path.GetFileName(PlatformHeaderFilename) + "\"\n");
		sourceFile.Write("#include \"Render/" + PlatformBase + "_Shader.h\"\n\n");
		sourceFile.Write("namespace Scaleform { namespace Render { namespace " + PlatformBase + " {\n\n");
		sourceFile.CurrentIndent = "";
	}

	protected override void writeSourcePostamble(IndentStreamWriter sourceFile)
	{
		sourceFile.Write("}}} // Scaleform::Render::" + PlatformBase + "\n\n");
		sourceFile.Write("#endif // WITH_SCALEFORM\n");
	}

	protected override void writeHeaderPipelineShaderDataMembers(IndentStreamWriter headerFile, ShaderPipeline pipeline)
	{
	}

	protected override void writeSourcePipelineShaderGlobals(IndentStreamWriter sourceFile, ShaderVersion ver, ShaderPipeline pipeline, string srcID)
	{
	}

	protected override void writeSourcePipelineShaderData(IndentStreamWriter sourceFile, ShaderVersion ver, ShaderPipeline pipeline, ShaderLinkedSource src)
	{
	}

	public override string GeneratePipelineHeaderExtras(ShaderPipeline pipeline)
	{
		string text = "";
		text += "const char*      pName;\n";
		if (pipeline.Type != ShaderPipeline.PipelineType.Vertex)
		{
			return text + base.GeneratePipelineHeaderExtras(pipeline);
		}
		int num = 0;
		foreach (ShaderVersion requestedShaderVersion in RequestedShaderVersions)
		{
			foreach (List<ShaderLinkedSource> value in requestedShaderVersion.LinkedSourceUniqueDescs.Values)
			{
				num = Math.Max(num, value.Max((ShaderLinkedSource src) => src.VariableList.FindAll((ShaderVariable v) => v.VarType == ShaderVariable.VariableType.Variable_Attribute || v.VarType == ShaderVariable.VariableType.Variable_VirtualAttribute).Count));
			}
		}
		text += "struct VertexAttrDesc\n";
		text += "{\n";
		text += "    const char*   Name;\n";
		text += "    unsigned      Attr;\n";
		text += "    unsigned      UsageIndex;\n";
		text += "};\n\n";
		text += "char           NumAttribs;\n";
		text += "enum {\n";
		object obj = text;
		text = string.Concat(obj, "    MaxVertexAttributes = ", num, "\n");
		text += "};\n";
		return text + "VertexAttrDesc Attributes[MaxVertexAttributes];\n";
	}

	public override string GeneratePipelineSourceExtras(ShaderVersion ver, ShaderPipeline pipeline, ShaderLinkedSource src)
	{
		string text = "";
		text = text + "/* pName      */    \"" + Path.GetFileNameWithoutExtension(ver.GetShaderDuplicateFilename(src)) + "\",\n";
		if (pipeline.Type != ShaderPipeline.PipelineType.Vertex)
		{
			return text + base.GeneratePipelineSourceExtras(ver, pipeline, src);
		}
		List<ShaderVariable> list = src.VariableList.FindAll((ShaderVariable v) => v.VarType == ShaderVariable.VariableType.Variable_Attribute);
		text = text + "/* NumAttribs */    " + list.Count + ",\n";
		text += "/* Attributes */    {\n";
		foreach (ShaderVariable item in list)
		{
			string text2 = "VET_Color";
			string semantic = item.Semantic;
			semantic = Regex.Replace(semantic, "\\d+$", "");
			string text3 = Regex.Replace(item.Semantic, "^[^0-9]+", "");
			text2 = semantic switch
			{
				"POSITION" => "VET_Pos", 
				"COLOR" => "VET_Color", 
				"TEXCOORD" => "VET_TexCoord", 
				"INSTANCE" => "VET_Instance8", 
				"FACTOR" => "VET_Color | (1 << VET_Index_Shift)", 
				_ => throw new Exception("Unexpected semantic: " + semantic), 
			};
			if (semantic == "INSTANCE" || semantic == "FACTOR")
			{
				text3 = "1";
			}
			object obj = text;
			text = string.Concat(obj, "{ \"", item.ID, "\", ".PadRight(13 - item.ID.Length), item.ElementCount, " | ", text2, ", ", text3, "},\n");
		}
		return text + "},\n";
	}

	public override void ShaderBuildEvent(ShaderBuildEventType eventType)
	{
		string option = CommandLineParser.GetOption(CommandLineOptions.UE4Path.ToString());
		if (option == null)
		{
			Console.WriteLine("UE4 path is not defined. Shaders in UE4 will not be cleaned or copied.");
			return;
		}
		option = option.Trim("\"".ToCharArray());
		switch (eventType)
		{
		case ShaderBuildEventType.ShaderBuildEvent_Initialize:
			if (!CommandLineParser.GetOption<bool>(CommandLineParser.Options.SkipHeaderAndSourceRegeneration))
			{
				DeleteFileWildcard(Path.Combine(option, "Engine\\Source\\Runtime\\ScaleformUI\\Private\\Render\\") + "\\\\RHI_ShaderDescs.*");
			}
			DeleteFileWildcard(Path.Combine(option, "Engine\\Shaders\\") + "\\\\GFx*.usf");
			DeleteFileWildcard(Path.Combine(option, "Engine\\Shaders\\Binaries\\") + "\\\\GFx*");
			DeleteFileWildcard(PlatformObjDirectory + "\\\\*.usf");
			break;
		case ShaderBuildEventType.ShaderBuildEvent_Finalize:
			if (!CommandLineParser.GetOption<bool>(CommandLineParser.Options.SkipHeaderAndSourceRegeneration))
			{
				CopyFileWildcard(CommandLineParser.GetOption(CommandLineParser.Options.OutputDirectory) + "\\\\RHI_ShaderDescs.*", Path.Combine(option, "Engine\\Source\\Runtime\\ScaleformUI\\Private\\Render"));
			}
			CopyFileWildcard(PlatformObjDirectory + "\\\\*.usf", Path.Combine(option, "Engine\\Shaders\\"));
			break;
		case ShaderBuildEventType.ShaderBuildEvent_PostShaderDesc:
			break;
		}
	}
}
