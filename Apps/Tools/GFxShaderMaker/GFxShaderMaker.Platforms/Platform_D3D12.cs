using System;
using System.Collections.Generic;
using System.Linq;
using System.Text.RegularExpressions;

namespace GFxShaderMaker.Platforms;

[Platform("D3D12", "Windows Direct3D12 HLSL")]
public class Platform_D3D12 : Platform_D3DCommon
{
	private List<ShaderVersion> ShaderVersions = new List<ShaderVersion>();

	protected List<ShaderVersion> PosShaderVersions;

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

	public override List<ShaderVersion> PossibleShaderVersions
	{
		get
		{
			if (PosShaderVersions != null)
			{
				return PosShaderVersions;
			}
			PosShaderVersions = new List<ShaderVersion>();
			PosShaderVersions.Add(new ShaderVersion_D3D12(this));
			return PosShaderVersions;
		}
	}

	public override List<ShaderVersion> RequestedShaderVersions => PossibleShaderVersions;

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
			text += "    const char*              Name;\n";
			text += "    unsigned                 Attr;\n";
			text += "};\n";
			text += "enum {\n";
			object obj = text;
			text = string.Concat(obj, "    MaxVertexAttributes = ", num, "\n");
			text += "};\n";
			text += "char                     NumAttribs;\n";
			text += "VertexAttrDesc           Attributes[MaxVertexAttributes];\n";
			text += "D3D12_INPUT_ELEMENT_DESC D3D12Attributes[MaxVertexAttributes];\n";
			break;
		}
		}
		return text;
	}

	public override string GeneratePipelineSourceExtras(ShaderVersion ver, ShaderPipeline pipeline, ShaderLinkedSource src)
	{
		string text = "";
		if (pipeline.Type == ShaderPipeline.PipelineType.Vertex)
		{
			List<ShaderVariable> sortedAttributeList = src.SortedAttributeList;
			if (src.PostFunctions.Find((string f) => f == "Instanced") != null)
			{
				sortedAttributeList.RemoveAll((ShaderVariable v) => v.Semantic.StartsWith("INSTANCE"));
			}
			text = text + "/* NumAttribs */    " + sortedAttributeList.Count + ",\n";
			text += "/* Attributes */    {\n";
			int num = 0;
			int num2 = 0;
			string text2 = "";
			foreach (ShaderVariable item in sortedAttributeList)
			{
				string text3 = "VET_Color";
				string semantic = item.Semantic;
				semantic = Regex.Replace(semantic, "\\d+$", "");
				string text4 = semantic;
				string text5 = Regex.Replace(item.Semantic, "^[^0-9]+", "");
				string text6 = "";
				int num3 = 0;
				switch (semantic)
				{
				default:
					throw new Exception("Unexpected semantic: " + semantic);
				case "POSITION":
					text6 = "DXGI_FORMAT_R32G32_FLOAT";
					text3 = "VET_Pos";
					if (ver.GetType() == typeof(ShaderVersion_D3D12))
					{
						text4 = "SV_Position";
					}
					num3 = 8;
					break;
				case "COLOR":
					text6 = "DXGI_FORMAT_R8G8B8A8_UNORM";
					text3 = "VET_Color";
					num3 = 4;
					break;
				case "TEXCOORD":
					text6 = "DXGI_FORMAT_R32G32_FLOAT";
					text3 = "VET_TexCoord";
					num3 = 8;
					break;
				case "INSTANCE":
					text6 = "DXGI_FORMAT_R8G8B8A8_UINT";
					text3 = "VET_Instance | VET_U8  | 4 | VET_Argument_Flag";
					num3 = 4;
					break;
				case "FACTOR":
					text6 = "DXGI_FORMAT_R8G8B8A8_UNORM";
					text3 = "VET_Color | (1 << VET_Index_Shift)";
					num3 = 4;
					break;
				}
				if (semantic == "INSTANCE" || semantic == "FACTOR")
				{
					text4 = "COLOR";
					text5 = "-1";
					List<ShaderVariable> list = sortedAttributeList.FindAll((ShaderVariable v) => v.VarType == ShaderVariable.VariableType.Variable_Attribute && v.Semantic.StartsWith("COLOR"));
					if (list.Count > 0)
					{
						text5 = Regex.Replace(list.Max((ShaderVariable v) => v.Semantic), "^.*(\\d+)$", "$1");
					}
					text5 = (Convert.ToInt32(text5) + ((semantic == "FACTOR") ? 1 : 2)).ToString();
				}
				object obj = text;
				text = string.Concat(obj, "{ \"", item.ID, "\", ".PadRight(13 - item.ID.Length), item.ElementCount, " | ", text3, "},\n");
				object obj2 = text2;
				text2 = string.Concat(obj2, "{ \"", text4, "\", ", text5, ", ", text6, ", 0, ", num2, ", D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },\n");
				num++;
				num2 += num3;
			}
			text += "},\n";
			text += "/* D3D12Attributes */    ";
			return text + "{\n" + text2 + "}\n";
		}
		return base.GeneratePipelineSourceExtras(ver, pipeline, src);
	}

	protected override void writeHeaderPreamble(IndentStreamWriter headerFile)
	{
		headerFile.Write("#include \"D3D12_Config.h\" // include D3D12.h for D3D12 structs\n");
		base.writeHeaderPreamble(headerFile);
		headerFile.Write("typedef unsigned char BYTE;\n");
	}
}
