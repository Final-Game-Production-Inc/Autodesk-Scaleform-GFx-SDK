using System;
using System.Collections.Generic;
using System.Linq;
using System.Text.RegularExpressions;

namespace GFxShaderMaker.Platforms;

public class ShaderVersion_PSVITA : ShaderVersion
{
	public override string SourceExtension => ".cg";

	public ShaderVersion_PSVITA(ShaderPlatform platform)
		: base(platform, "PSVita")
	{
	}

	public override string GetVariableUniformRegisterType(ShaderVariable var)
	{
		string result = "c";
		if (Regex.Matches(var.Type, "sampler", RegexOptions.IgnoreCase).Count > 0)
		{
			result = "s";
		}
		return result;
	}

	public override void PostLink_Instanced(ShaderLinkedSource linkedSrc)
	{
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
			text = text3 + ((var.VarType == outType) ? "out " : "") + type + " " + var.ID + ((var.ArraySize > 1) ? ("[" + var.ArraySize + "]") : "");
			if (var.VarType != ShaderVariable.VariableType.Variable_Attribute)
			{
				text = text + " : " + text2;
			}
		}
		text += ")\n{";
		text = text + linkedSrc.SourceCode + "}\n";
		text = text.Replace("lowpf", "float");
		text = Regex.Replace(text, "mul\\s*\\((?'P0'" + ShaderVersion.SubexprRegex + "),(?'P1'" + ShaderVersion.SubexprRegex + ")\\)", "mul(${P1}, ${P0})");
		text = Regex.Replace(text, "tex2Dlod\\s*\\(([^,]+),([^,]+),(.+)\\)", "tex2Dlod( $1, float4( ($2), 0.0, $3 ) )", RegexOptions.IgnoreCase);
		text = Regex.Replace(text, "(\\d+)f", "$1");
		text = Regex.Replace(text, "\\.f", "\\.");
		if (linkedSrc.Pipeline.Type == ShaderPipeline.PipelineType.Fragment)
		{
			text = Regex.Replace(text, "\\bfloat([1-4])?\\b", "half$1");
		}
		return text;
	}
}
