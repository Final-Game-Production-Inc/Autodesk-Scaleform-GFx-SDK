using System;
using System.Collections.Generic;
using System.Linq;
using System.Text.RegularExpressions;

namespace GFxShaderMaker.Platforms;

public class ShaderVersion_PS3 : ShaderVersion
{
	public override string SourceExtension => ".cg";

	public ShaderVersion_PS3(ShaderPlatform platform)
		: base(platform, "PS3")
	{
	}

	public override string GetVariableUniformRegisterType(ShaderVariable var)
	{
		return "c";
	}

	public override void PostLink_Instanced(ShaderLinkedSource linkedSrc)
	{
	}

	public override string CreateFinalSource(ShaderLinkedSource linkedSrc)
	{
		string text = "";
		string text2 = linkedSrc.SourceCode;
		List<ShaderVariable> list = linkedSrc.VariableList.FindAll((ShaderVariable shaderVariable) => shaderVariable.VarType == ShaderVariable.VariableType.Variable_Uniform).ToList();
		list.Sort();
		foreach (ShaderVariable item in list)
		{
			if (item.ArraySize > 1 && linkedSrc.Pipeline.Type == ShaderPipeline.PipelineType.Fragment)
			{
				for (uint num = 0u; num < item.ArraySize; num++)
				{
					object obj = text;
					text = string.Concat(obj, item.Type, " ", item.ID, (num != 0) ? num.ToString() : "", " : register(", GetVariableUniformRegisterType(item), item.BaseRegister + num * item.RegisterCountPerElement, ");\n");
					text2 = Regex.Replace(text2, item.ID + "\\[\\s*" + num + "\\s*\\]", item.ID + ((num != 0) ? num.ToString() : ""));
					if (!item.SamplerType)
					{
						throw new Exception("PS3 cannot handle non-sampler uniform arrays, shader must be refactored. VariableName = " + item.ID + " in shader " + linkedSrc.ID);
					}
				}
			}
			else
			{
				object obj2 = text;
				text = string.Concat(obj2, item.Type, " ", item.ID, (item.ArraySize > 1) ? ("[" + item.ArraySize + "]") : "", " : register(", GetVariableUniformRegisterType(item), item.BaseRegister, ");\n");
			}
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
			string text3 = var.Semantic;
			string type = var.Type;
			if (var.Semantic.StartsWith("FACTOR") || var.Semantic.StartsWith("INSTANCE"))
			{
				text3 = "COLOR";
				List<ShaderVariable> list2 = linkedSrc.VariableList.FindAll((ShaderVariable v) => v.Semantic.StartsWith("COLOR") && v.VarType == var.VarType);
				string value = "-1";
				if (list2.Count > 0)
				{
					value = Regex.Replace(list2.Max((ShaderVariable v) => v.Semantic), "^.*(\\d+)$", "$1");
				}
				text3 += Convert.ToInt32(value) + (var.Semantic.StartsWith("FACTOR") ? 1 : 2);
			}
			string text4 = text;
			text = text4 + ((var.VarType == outType) ? "out " : "") + type + " " + var.ID + ((var.ArraySize > 1) ? ("[" + var.ArraySize + "]") : "");
			if (var.VarType != ShaderVariable.VariableType.Variable_Attribute)
			{
				text = text + " : " + text3;
			}
		}
		text += ")\n{";
		text = text + text2 + "}\n";
		text = text.Replace("lowpf", "float");
		text = Regex.Replace(text, "mul\\s*\\((?'P0'" + ShaderVersion.SubexprRegex + "),(?'P1'" + ShaderVersion.SubexprRegex + ")\\)", "mul(${P1}, ${P0})");
		return Regex.Replace(text, "tex2Dlod\\s*\\(([^,]+),([^,]+),(.+)\\)", "tex2Dlod( $1, float4( ($2), 0.0, $3 ) )", RegexOptions.IgnoreCase);
	}
}
