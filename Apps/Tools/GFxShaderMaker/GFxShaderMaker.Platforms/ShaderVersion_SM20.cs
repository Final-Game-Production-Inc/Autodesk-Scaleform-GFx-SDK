using System;
using System.Collections.Generic;
using System.Linq;
using System.Text.RegularExpressions;

namespace GFxShaderMaker.Platforms;

internal class ShaderVersion_SM20 : ShaderVersion_D3DCommon
{
	private List<string> UnsupportedFlagsSM20;

	public override List<string> UnsupportedFlags => UnsupportedFlagsSM20;

	public ShaderVersion_SM20(ShaderPlatform platform, string id)
		: base(platform, id)
	{
		UnsupportedFlagsSM20 = new List<string>();
		UnsupportedFlagsSM20.Add("Instanced");
		UnsupportedFlagsSM20.Add("DynamicLoop");
		UnsupportedFlagsSM20.Add("Derivatives");
	}

	public override string GetShaderProfile(ShaderPipeline pipeline)
	{
		return pipeline.Type switch
		{
			ShaderPipeline.PipelineType.Fragment => "ps_2_0", 
			ShaderPipeline.PipelineType.Vertex => "vs_2_0", 
			_ => null, 
		};
	}

	public override string CreateFinalSource(ShaderLinkedSource linkedSrc)
	{
		string text = "";
		List<ShaderVariable> list = linkedSrc.VariableList.FindAll((ShaderVariable shaderVariable) => shaderVariable.VarType == ShaderVariable.VariableType.Variable_Uniform).ToList();
		list.Sort();
		foreach (ShaderVariable item in list)
		{
			object obj = text;
			text = string.Concat(obj, item.Type, " ", item.ID, (item.ArraySize > 1) ? ("[" + item.ArraySize + "]") : "", " : register(", GetVariableUniformRegisterType(item), item.BaseRegister, ");\n");
		}
		text += "void main( ";
		bool flag = true;
		ShaderVariable.VariableType inType;
		ShaderVariable.VariableType outType;
		switch (linkedSrc.Pipeline.Type)
		{
		default:
			inType = ShaderVariable.VariableType.Variable_Attribute;
			outType = ShaderVariable.VariableType.Variable_Varying;
			break;
		case ShaderPipeline.PipelineType.Fragment:
			inType = ShaderVariable.VariableType.Variable_Varying;
			outType = ShaderVariable.VariableType.Variable_FragOut;
			break;
		}
		ShaderVariable var;
		foreach (ShaderVariable item2 in linkedSrc.VariableList.FindAll((ShaderVariable shaderVariable) => shaderVariable.VarType == inType || shaderVariable.VarType == outType))
		{
			var = item2;
			if (!flag)
			{
				text += ",\n           ";
			}
			flag = false;
			string text2 = var.Semantic;
			string type = var.Type;
			if (var.Semantic.StartsWith("FACTOR") || var.Semantic.StartsWith("INSTANCE"))
			{
				text2 = "COLOR";
				List<ShaderVariable> list2 = linkedSrc.VariableList.FindAll((ShaderVariable v) => v.Semantic.StartsWith("COLOR") && v.VarType == var.VarType);
				string value = "-1";
				if (list2.Count > 0)
				{
					value = Regex.Replace(list2.Max((ShaderVariable v) => v.Semantic), "^.*(\\d+)$", "$1");
				}
				text2 += Convert.ToInt32(value) + (var.Semantic.StartsWith("FACTOR") ? 1 : 2);
			}
			string text3 = text;
			text = text3 + ((var.VarType == outType) ? "out " : "") + type + " " + var.ID + ((var.ArraySize > 1) ? ("[" + var.ArraySize + "]") : "") + " : " + text2;
		}
		text += ")\n{";
		text = text + linkedSrc.SourceCode + "}\n";
		text = text.Replace("lowpf", "float");
		text = text.Replace("half", "float");
		text = Regex.Replace(text, "\\bdiscard\\b", "clip(-1)");
		text = Regex.Replace(text, "tex2Dlod\\s*\\(([^,]+),([^,]+),(.+)\\)", "tex2Dlod( $1, float4( ($2), 0.0, $3 ) )", RegexOptions.IgnoreCase);
		if (linkedSrc.Flags.Contains("DynamicLoop"))
		{
			text = Regex.Replace(text, "tex2D\\s*\\((?'P0'" + ShaderVersion.SubexprRegex + "),(?'P1'" + ShaderVersion.SubexprRegex + ")\\)", "tex2Dlod(${P0}, float4((${P1}).xy, 0.0f, 0.0f))");
		}
		return text;
	}
}
