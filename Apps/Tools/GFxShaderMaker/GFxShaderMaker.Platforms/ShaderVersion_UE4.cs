using System.Collections.Generic;
using System.Linq;
using System.Text.RegularExpressions;

namespace GFxShaderMaker.Platforms;

public class ShaderVersion_UE4 : ShaderVersion
{
	public override List<string> UnsupportedFlags
	{
		get
		{
			List<string> list = new List<string>();
			list.Add("Instanced");
			return list;
		}
	}

	public override string SourceExtension => ".usf";

	public ShaderVersion_UE4(ShaderPlatform platform, string id = "UE4")
		: base(platform, id)
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
		base.PostLink_Batch(linkedSrc);
		UndoFactorBatchIndexing(linkedSrc);
	}

	public override string CreateFinalSource(ShaderLinkedSource linkedSrc)
	{
		string text = "";
		string text2 = "";
		text2 += "#include \"Common.usf\"\n";
		text2 += "#if PS4_PROFILE\n";
		text2 += "\t#define SV_Position\tS_POSITION\n";
		text2 += "\t#define pow(x, y)\tpow(float3(x), float(y))\n";
		text2 += "#endif\n\n";
		List<ShaderVariable> list = linkedSrc.VariableList.FindAll((ShaderVariable var) => var.VarType == ShaderVariable.VariableType.Variable_Uniform && !var.SamplerType).ToList();
		List<ShaderVariable> list2 = linkedSrc.VariableList.FindAll((ShaderVariable var) => var.VarType == ShaderVariable.VariableType.Variable_Uniform && var.SamplerType).ToList();
		list.Sort();
		foreach (ShaderVariable item in list)
		{
			string text3 = text2;
			text2 = text3 + item.Type + " " + item.ID + ((item.ArraySize > 1) ? ("[" + item.ArraySize + "]") : "") + ";\n";
		}
		foreach (ShaderVariable item2 in list2)
		{
			if (item2.ArraySize > 1)
			{
				text2 += "#if METAL_PROFILE\n";
				for (int num = 0; num < item2.ArraySize; num++)
				{
					object obj = text2;
					text2 = string.Concat(obj, "\tSAMPLER2D(", item2.ID, num, ");\n");
					object obj2 = text2;
					text2 = string.Concat(obj2, "\t#define SVAR", num, "\t", item2.ID, num, "\n");
					object obj3 = text2;
					text2 = string.Concat(obj3, "\t#define SVAR", num, "SAMPLER\t", item2.ID, num, "Sampler\n");
				}
				text2 += "#else\n";
				string text4 = text2;
				text2 = text4 + "\tSAMPLER2DARRAY(" + item2.ID + ", " + item2.ArraySize + ");\n";
				for (int num2 = 0; num2 < item2.ArraySize; num2++)
				{
					object obj4 = text2;
					text2 = string.Concat(obj4, "\t#define SVAR", num2, "\t", item2.ID, "[", num2, "]\n");
					object obj5 = text2;
					text2 = string.Concat(obj5, "\t#define SVAR", num2, "SAMPLER\t", item2.ID, "Sampler[", num2, "]\n");
				}
				text2 += "#endif\n";
			}
			else
			{
				text2 = text2 + "SAMPLER2D(" + item2.ID + ");\n";
			}
		}
		text += "\n\nvoid main( ";
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
		foreach (ShaderVariable item3 in linkedSrc.VariableList.FindAll((ShaderVariable var) => var.VarType == inType || var.VarType == outType))
		{
			if (!flag)
			{
				text += ",\n           ";
			}
			flag = false;
			string text5 = item3.Semantic;
			string text6 = item3.Type;
			if (item3.Semantic.StartsWith("FACTOR") || item3.Semantic.StartsWith("INSTANCE"))
			{
				text5 = "COLOR1";
			}
			if (item3.ID == "vbatch")
			{
				text5 = "BLENDINDICES0";
			}
			if (item3.Semantic.StartsWith("INSTANCE") || text5.StartsWith("SV_InstanceID"))
			{
				text6 = Regex.Replace(text6, "float", "uint");
				text6 = Regex.Replace(text6, "lowpf", "uint");
			}
			if (text5.StartsWith("TEXCOORD"))
			{
				int num3 = int.Parse(text5.Substring("TEXCOORD".Length));
				text5 = "ATTRIBUTE" + (4 + num3);
			}
			else if (text5.StartsWith("COLOR"))
			{
				if (item3.VarType == outType && linkedSrc.Pipeline.Type == ShaderPipeline.PipelineType.Fragment)
				{
					text5 = "SV_Target0";
				}
				else
				{
					int num4 = int.Parse(text5.Substring("COLOR".Length));
					text5 = "ATTRIBUTE" + (2 + num4);
				}
			}
			else if (text5.Equals("BLENDINDICES0"))
			{
				text5 = "ATTRIBUTE1";
			}
			else if (text5.Equals("POSITION0"))
			{
				text5 = ((item3.VarType != outType) ? "ATTRIBUTE0" : "SV_Position");
			}
			string text7 = text;
			text = text7 + ((item3.VarType == outType) ? "out " : "") + text6 + " " + item3.ID + ((item3.ArraySize > 1) ? ("[" + item3.ArraySize + "]") : "") + " : " + text5;
		}
		text += ")\n{";
		text += linkedSrc.SourceCode;
		text += "}\n";
		text2 = text2.Replace("lowpf", "float");
		text2 = text2.Replace("half", "float");
		text = text.Replace("lowpf", "float");
		text = text.Replace("half", "float");
		text = Regex.Replace(text, "\\bdiscard\\b", "clip(-1)");
		text = Regex.Replace(text, "tex2Dlod\\s*\\(([^,]+),([^,]+),(.+)\\)", "tex2Dlod( $1, float4( ($2), 0.0, $3 ) )", RegexOptions.IgnoreCase);
		foreach (ShaderVariable item4 in list2)
		{
			for (int num5 = 0; num5 < item4.ArraySize; num5++)
			{
				string oldValue = "tex2D(" + item4.ID + "[" + num5 + "],";
				string oldValue2 = "tex2Dlod(" + item4.ID + "[" + num5 + "],";
				string newValue = "tex2Dsamp(SVAR" + num5 + ", SVAR" + num5 + "SAMPLER,";
				text = text.Replace(oldValue, newValue);
				text = text.Replace(oldValue2, newValue);
			}
		}
		text = Regex.Replace(text, "\\[([^\\]]+)\\]", "[int($1)]");
		text2 = Regex.Replace(text2, "\\[([^\\]]+)\\]", "[int($1)]");
		return text2 + text;
	}

	public override string GetShaderFilename(ShaderLinkedSource src)
	{
		return "GFx_" + src.ID + SourceExtension;
	}

	public override string GetShaderDuplicateFilename(ShaderLinkedSource src)
	{
		return "GFx_" + src.SourceCodeDuplicateID + SourceExtension;
	}
}
