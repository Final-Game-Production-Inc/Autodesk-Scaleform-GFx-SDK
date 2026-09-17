using System;
using System.Collections.Generic;
using System.Linq;
using System.Text.RegularExpressions;

namespace GFxShaderMaker.Platforms;

[Platform("D3D1x", "Windows Direct3D10/11 HLSL")]
public class Platform_D3D1x : Platform_D3DCommon
{
	public enum CommandLineOptions
	{
		[CommandLineOption(CmdLineOptionType.OptionType_String, "featurelevels", null, "The comma separated list of shader model(s) required (see -listfl).", null)]
		FeatureLevel,
		[CommandLineOption(CmdLineOptionType.OptionType_Action, "listfl", null, "Lists the possible feature levels (use with -featureLevels)", typeof(ListD3D1xFeatureLevelAction))]
		ListFeatureLevels
	}

	public enum FeatureLevels
	{
		[ShaderVersion(typeof(ShaderVersion_D3D1x_FL91))]
		D3D_FEATURE_LEVEL_9_1,
		[ShaderVersion(typeof(ShaderVersion_D3D1x_FL93))]
		D3D_FEATURE_LEVEL_9_3,
		[ShaderVersion(typeof(ShaderVersion_D3D1x_FL10_X))]
		D3D_FEATURE_LEVEL_10_0,
		[ShaderVersion(typeof(ShaderVersion_D3D1x_FL11_X))]
		D3D_FEATURE_LEVEL_11_0
	}

	protected List<ShaderVersion> ReqShaderVersions;

	protected List<ShaderVersion> PosShaderVersions;

	public override List<ShaderVersion> PossibleShaderVersions
	{
		get
		{
			if (PosShaderVersions != null)
			{
				return PosShaderVersions;
			}
			PosShaderVersions = ExtractPossibleVersions(typeof(FeatureLevels), null);
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
			string option = CommandLineParser.GetOption<string>(CommandLineOptions.FeatureLevel.ToString());
			ReqShaderVersions = ExtractPossibleVersions(typeof(FeatureLevels), option, PossibleShaderVersions);
			return ReqShaderVersions;
		}
	}

	protected override string D3DSDKEnvironmentVariable => "DXSDK_DIR";

	protected override string D3DFXCExtraOptions => "/O3";

	protected override string GetBinaryShaderDeclaration(ShaderPipeline pipeline)
	{
		return "const BYTE *     pBinary;\nint              BinarySize;";
	}

	protected override string GetBinaryShaderReference(ShaderPipeline pipeline, string id)
	{
		return string.Format("pBinary_{0},\npBinary_{0}_size,", id);
	}

	protected override string GetBinaryShaderExtern(ShaderPipeline pipeline, string id)
	{
		return string.Format("extern const BYTE   pBinary_{0}[];\nextern const int    pBinary_{0}_size;", id);
	}

	public override string GeneratePipelineHeaderExtras(ShaderPipeline pipeline)
	{
		string text = "";
		switch (pipeline.Type)
		{
		case ShaderPipeline.PipelineType.Fragment:
			text = base.GeneratePipelineHeaderExtras(pipeline);
			break;
		case ShaderPipeline.PipelineType.Vertex:
		{
			int num = 0;
			foreach (ShaderVersion requestedShaderVersion in RequestedShaderVersions)
			{
				foreach (List<ShaderLinkedSource> value in requestedShaderVersion.LinkedSourceUniqueDescs.Values)
				{
					num = Math.Max(num, value.Max((ShaderLinkedSource src) => src.VariableList.FindAll((ShaderVariable v) => v.VarType == ShaderVariable.VariableType.Variable_Attribute).Count));
				}
			}
			text += "struct VertexAttrDesc\n";
			text += "{\n";
			text += "    const char*   Name;\n";
			text += "    unsigned      Attr;\n";
			text += "    const char*   SemanticName;\n";
			text += "    unsigned      SemanticIndex;\n";
			text += "    unsigned      Format; // DXGI_FORMAT\n";
			text += "};\n\n";
			text += "char           NumAttribs;\n";
			text += "enum {\n";
			object obj = text;
			text = string.Concat(obj, "    MaxVertexAttributes = ", num, "\n");
			text += "};\n";
			object obj2 = text;
			text = string.Concat(obj2, "VertexAttrDesc Attributes[", num, "];\n");
			break;
		}
		}
		return text;
	}

	public override string GeneratePipelineSourceExtras(ShaderVersion ver, ShaderPipeline pipeline, ShaderLinkedSource src)
	{
		string text = "";
		string text2 = "";
		if (pipeline.Type == ShaderPipeline.PipelineType.Vertex)
		{
			List<ShaderVariable> list = src.VariableList.FindAll((ShaderVariable v) => v.VarType == ShaderVariable.VariableType.Variable_Attribute);
			if (src.PostFunctions.Find((string f) => f == "Instanced") != null)
			{
				list.RemoveAll((ShaderVariable v) => v.Semantic.StartsWith("INSTANCE"));
			}
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
				string text7 = "";
				switch (semantic)
				{
				default:
					throw new Exception("Unexpected semantic: " + semantic);
				case "POSITION":
					text7 = "DXGI_FORMAT_R32G32_FLOAT";
					text4 = "VET_Pos";
					if (ver.GetType() == typeof(ShaderVersion_D3D1x_FL10_X) || ver.GetType() == typeof(ShaderVersion_D3D1x_FL11_X))
					{
						text5 = "SV_Position";
					}
					break;
				case "COLOR":
					text7 = "DXGI_FORMAT_R8G8B8A8_UNORM";
					text4 = "VET_Color";
					break;
				case "TEXCOORD":
					text7 = "DXGI_FORMAT_R32G32_FLOAT";
					text4 = "VET_TexCoord";
					break;
				case "INSTANCE":
					if (ver.GetType() == typeof(ShaderVersion_D3D1x_FL10_X) || ver.GetType() == typeof(ShaderVersion_D3D1x_FL11_X))
					{
						text7 = "DXGI_FORMAT_R8_UINT";
						text4 = "VET_Instance8";
					}
					else
					{
						text7 = "DXGI_FORMAT_R8G8B8A8_UNORM";
						text4 = "VET_Instance | VET_U8  | 4 | VET_Argument_Flag";
					}
					break;
				case "FACTOR":
					text7 = "DXGI_FORMAT_R8G8B8A8_UNORM";
					text4 = "VET_Color | (1 << VET_Index_Shift)";
					break;
				}
				if (semantic == "INSTANCE" || semantic == "FACTOR")
				{
					text5 = "COLOR";
					text6 = "-1";
					List<ShaderVariable> list2 = list.FindAll((ShaderVariable v) => v.VarType == ShaderVariable.VariableType.Variable_Attribute && v.Semantic.StartsWith("COLOR"));
					if (list2.Count > 0)
					{
						text6 = Regex.Replace(list2.Max((ShaderVariable v) => v.Semantic), "^.*(\\d+)$", "$1");
					}
					text6 = (Convert.ToInt32(text6) + ((semantic == "FACTOR") ? 1 : 2)).ToString();
				}
				object obj = text;
				text = string.Concat(obj, text2, "{ \"", item.ID, "\", ".PadRight(13 - item.ID.Length), item.ElementCount, " | ", text4, ", \"", text5, "\", ", text6, ", ", text7, "},\n");
			}
			text = text + text2 + "},\n";
			text2 = text2.Remove(0, 22);
		}
		else
		{
			text = base.GeneratePipelineSourceExtras(ver, pipeline, src);
		}
		return text;
	}

	protected override void writeHeaderPreamble(IndentStreamWriter headerFile)
	{
		base.writeHeaderPreamble(headerFile);
		headerFile.Write("typedef unsigned char BYTE;\n");
	}
}
