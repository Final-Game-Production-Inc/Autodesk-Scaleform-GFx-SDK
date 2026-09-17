using System;
using System.Collections.Generic;
using System.Linq;
using System.Text.RegularExpressions;

namespace GFxShaderMaker.Platforms;

[Platform("X360", "Windows Direct3D9 HLSL")]
public class Platform_X360 : Platform_D3DCommon
{
	private List<ShaderVersion> ShaderVersions = new List<ShaderVersion>();

	public override List<ShaderVersion> RequestedShaderVersions => ShaderVersions;

	protected override string D3DFXCExtraOptions
	{
		get
		{
			string option = CommandLineParser.GetOption(CommandLineParser.Options.Config);
			switch (option)
			{
			case "Debug":
			case "DebugOpt":
				return "/XZi";
			case "Release":
				return "/XZi";
			case "Shipping":
				return "";
			default:
				throw new Exception("Unsupported configuration type: " + option);
			}
		}
	}

	protected override bool PreferWindowsKitsFXC => false;

	protected override string D3DSDKEnvironmentVariable => "XEDK";

	public Platform_X360()
	{
		ShaderVersions.Add(new ShaderVersion_X360(this));
	}

	protected override string GetBinaryShaderDeclaration(ShaderPipeline pipeline)
	{
		return "const DWORD*     pBinary;\n";
	}

	protected override string GetBinaryShaderReference(ShaderPipeline pipeline, string id)
	{
		return $"pBinary_{id},";
	}

	protected override string GetBinaryShaderExtern(ShaderPipeline pipeline, string id)
	{
		return $"extern const DWORD pBinary_{id}[];";
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
				num = Math.Max(num, value.Max((ShaderLinkedSource src) => src.VariableList.FindAll((ShaderVariable v) => v.VarType == ShaderVariable.VariableType.Variable_Attribute || v.VarType == ShaderVariable.VariableType.Variable_VirtualAttribute).Count));
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
		object obj = text;
		return string.Concat(obj, "VertexAttrDesc Attributes[", num, "];\n");
	}

	public override string GeneratePipelineSourceExtras(ShaderVersion ver, ShaderPipeline pipeline, ShaderLinkedSource src)
	{
		string text = "";
		string text2 = "";
		if (pipeline.Type == ShaderPipeline.PipelineType.Fragment)
		{
			return base.GeneratePipelineSourceExtras(ver, pipeline, src);
		}
		List<ShaderVariable> list = src.VariableList.FindAll((ShaderVariable v) => v.VarType == ShaderVariable.VariableType.Variable_Attribute || v.VarType == ShaderVariable.VariableType.Variable_VirtualAttribute);
		bool flag = src.PostFunctions.Find((string f) => f == "Instanced") != null;
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
			switch (semantic)
			{
			default:
				throw new Exception("Unexpected semantic: " + semantic);
			case "POSITION":
				text4 = "VET_Pos";
				break;
			case "COLOR":
				text4 = "VET_Color";
				break;
			case "TEXCOORD":
				text4 = "VET_TexCoord";
				break;
			case "FACTOR":
				text4 = "VET_Color | (1 << VET_Index_Shift)";
				break;
			case "INSTANCE":
				if (flag)
				{
					continue;
				}
				text4 = "VET_Instance8";
				break;
			case "INDEX":
				text5 = "COLOR";
				text6 = "7";
				text4 = "VET_Instance8";
				break;
			}
			if (semantic == "INSTANCE" || semantic == "FACTOR")
			{
				text6 = "-1";
				text5 = "COLOR";
				List<ShaderVariable> list2 = list.FindAll((ShaderVariable v) => (v.VarType == ShaderVariable.VariableType.Variable_Attribute || v.VarType == ShaderVariable.VariableType.Variable_VirtualAttribute) && v.Semantic.StartsWith("COLOR"));
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
