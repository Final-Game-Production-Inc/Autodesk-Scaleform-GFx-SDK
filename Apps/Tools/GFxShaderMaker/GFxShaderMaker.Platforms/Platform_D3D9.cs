using System;
using System.Collections.Generic;
using System.Linq;
using System.Text.RegularExpressions;

namespace GFxShaderMaker.Platforms;

[Platform("D3D9", "Windows Direct3D9 HLSL")]
public class Platform_D3D9 : Platform_D3DCommon
{
	public enum CommandLineOptions
	{
		[CommandLineOption(CmdLineOptionType.OptionType_String, "shadermodel", null, "The comma separated list of shader model(s) required (see -listsms).", null)]
		ShaderModel,
		[CommandLineOption(CmdLineOptionType.OptionType_Action, "listsms", null, "Lists the possible shader models (use with -shadermodel)", typeof(ListD3D9ShaderModelsAction))]
		ListShaderModels
	}

	public enum ShaderModels
	{
		[ShaderVersion(typeof(ShaderVersion_D3D9_SM20))]
		SM20,
		[ShaderVersion(typeof(ShaderVersion_D3D9_SM30))]
		SM30
	}

	private List<ShaderVersion> ReqShaderVersions;

	private List<ShaderVersion> PosShaderVersions;

	public override List<ShaderVersion> PossibleShaderVersions
	{
		get
		{
			if (PosShaderVersions != null)
			{
				return PosShaderVersions;
			}
			PosShaderVersions = ExtractPossibleVersions(typeof(ShaderModels), null);
			return PosShaderVersions;
		}
	}

	public override List<ShaderVersion> RequestedShaderVersions
	{
		get
		{
			if (ReqShaderVersions != null)
			{
				return ReqShaderVersions;
			}
			string option = CommandLineParser.GetOption<string>(CommandLineOptions.ShaderModel.ToString());
			ReqShaderVersions = ExtractPossibleVersions(typeof(ShaderModels), option, PossibleShaderVersions);
			return ReqShaderVersions;
		}
	}

	protected override string D3DSDKEnvironmentVariable => "DXSDK_DIR";

	protected override string D3DFXCExtraOptions
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

	protected override void writeHeaderPreamble(IndentStreamWriter headerFile)
	{
		base.writeHeaderPreamble(headerFile);
		headerFile.Write("typedef unsigned char BYTE;\n");
	}

	public override string GeneratePipelineHeaderExtras(ShaderPipeline pipeline)
	{
		if (pipeline.Type == ShaderPipeline.PipelineType.Fragment)
		{
			return base.GeneratePipelineHeaderExtras(pipeline);
		}
		int num = 0;
		foreach (ShaderVersion requestedShaderVersion in RequestedShaderVersions)
		{
			foreach (List<ShaderLinkedSource> value in requestedShaderVersion.LinkedSourceUniqueDescs.Values)
			{
				num = Math.Max(num, value.Max((ShaderLinkedSource src) => src.VariableList.FindAll((ShaderVariable v) => v.VarType == ShaderVariable.VariableType.Variable_Attribute).Count));
			}
		}
		string text = "";
		text += "struct VertexAttrDesc\n";
		text += "{\n";
		text += "    const char*   Name;\n";
		text += "    unsigned      Attr;\n";
		text += "    unsigned char Usage;      /* D3DDECLUSAGE */\n";
		text += "    unsigned char UsageIndex; /* D3DDECLUSAGE_INDEX */\n";
		text += "};\n\n";
		text += "char           NumAttribs;\n";
		text += "enum {\n";
		object obj = text;
		text = string.Concat(obj, "    MaxVertexAttributes = ", num, "\n");
		text += "};\n";
		object obj2 = text;
		return string.Concat(obj2, "VertexAttrDesc Attributes[", num, "];\n");
	}

	public override string GeneratePipelineSourceExtras(ShaderVersion ver, ShaderPipeline pipeline, ShaderLinkedSource src)
	{
		string text = "";
		string text2 = "";
		if (pipeline.Type == ShaderPipeline.PipelineType.Fragment)
		{
			return base.GeneratePipelineSourceExtras(ver, pipeline, src);
		}
		List<ShaderVariable> list = src.VariableList.FindAll((ShaderVariable v) => v.VarType == ShaderVariable.VariableType.Variable_Attribute);
		src.PostFunctions.Find((string f) => f == "Instanced");
		string text3 = text;
		text = text3 + text2 + "/* NumAttribs */    " + list.Count + ",\n";
		text = text + text2 + "/* Attributes */    {\n";
		text2 += "                      ";
		foreach (ShaderVariable item in list)
		{
			string text4 = "VET_Color";
			string semantic = item.Semantic;
			semantic = Regex.Replace(semantic, "\\d+$", "");
			string text5 = semantic;
			string text6 = Regex.Replace(item.Semantic, "^[^0-9]+", "");
			text4 = semantic switch
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
				text6 = "-1";
				text5 = "COLOR";
				List<ShaderVariable> list2 = list.FindAll((ShaderVariable v) => v.VarType == ShaderVariable.VariableType.Variable_Attribute && v.Semantic.StartsWith("COLOR"));
				if (list2.Count > 0)
				{
					text6 = Regex.Replace(list2.Max((ShaderVariable v) => v.Semantic), "^.*(\\d+)$", "$1");
				}
				text6 = (Convert.ToInt32(text6) + ((semantic == "FACTOR") ? 1 : 2)).ToString();
			}
			object obj = text;
			text = string.Concat(obj, text2, "{ \"", item.ID, "\", ".PadRight(13 - item.ID.Length), item.ElementCount, " | ", text4, ", D3DDECLUSAGE_", text5, ", ", text6, "},\n");
		}
		text = text + text2 + "},\n";
		text2 = text2.Remove(0, 22);
		return text;
	}
}
