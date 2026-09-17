using System;
using System.Collections.Generic;
using System.Linq;
using System.Text.RegularExpressions;

namespace GFxShaderMaker.Platforms;

public class ShaderVersion_UE3 : ShaderVersion
{
	public bool GUsingMobileRHI { get; set; }

	public override List<string> UnsupportedFlags
	{
		get
		{
			List<string> list = new List<string>();
			list.Add("Instanced");
			return list;
		}
	}

	public override string SourceExtension
	{
		get
		{
			if (!GUsingMobileRHI)
			{
				return ".usf";
			}
			return ".msf";
		}
	}

	public ShaderVersion_UE3(ShaderPlatform platform, string id = "UE3")
		: base(platform, id)
	{
		GUsingMobileRHI = false;
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
		string text = "--USF--\n";
		text += createFinalSourceRHI(linkedSrc);
		text += "--USF--\n--MSF--\n";
		text += createFinalSourceMobile(linkedSrc);
		return text + "--MSF--\n";
	}

	private string createFinalSourceRHI(ShaderLinkedSource linkedSrc)
	{
		string text = "";
		string sourceCode = linkedSrc.SourceCode;
		text += "#include \"Common.usf\"\n";
		text += "#if !SM4_PROFILE && !SM5_PROFILE\n";
		text += "    #define int float\n";
		text += "#endif\n";
		text += "\n";
		List<ShaderVariable> list = linkedSrc.VariableList.FindAll((ShaderVariable var) => var.VarType == ShaderVariable.VariableType.Variable_Uniform).ToList();
		list.Sort();
		foreach (ShaderVariable item in list)
		{
			string text2 = text;
			text = text2 + item.Type + " " + item.ID + ((item.ArraySize > 1) ? ("[" + item.ArraySize + "]") : "") + ";\n";
		}
		text += "void main ( ";
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
		foreach (ShaderVariable item2 in linkedSrc.VariableList.FindAll((ShaderVariable var) => var.VarType == inType || var.VarType == outType))
		{
			if (!flag)
			{
				text += ",\n           ";
			}
			flag = false;
			string text3 = item2.Semantic;
			string text4 = item2.Type;
			if (item2.Semantic.StartsWith("FACTOR") || item2.Semantic.StartsWith("INSTANCE"))
			{
				text3 = "COLOR1";
			}
			if (item2.Semantic.StartsWith("INSTANCE") || text3.StartsWith("SV_InstanceID"))
			{
				text4 = Regex.Replace(text4, "float", "uint");
				text4 = Regex.Replace(text4, "lowpf", "uint");
			}
			string text5 = text;
			text = text5 + ((item2.VarType == outType) ? "out " : "") + text4 + " " + item2.ID + ((item2.ArraySize > 1) ? ("[" + item2.ArraySize + "]") : "") + " : " + text3;
		}
		sourceCode = Regex.Replace(sourceCode, "\\bdiscard\\b", "clip(-1)");
		sourceCode = Regex.Replace(sourceCode, "tex2Dlod\\s*\\(([^,]+),([^,]+),(.+)\\)", "tex2Dlod( $1, float4( ($2), 0.0, $3 ) )", RegexOptions.IgnoreCase);
		sourceCode = Regex.Replace(sourceCode, "(^|\\b)mul\\s*((?'P0'" + ShaderVersion.SubexprRegex + "),(?'P1'" + ShaderVersion.SubexprRegex + "))", "MulMatrix(${P0},${P1})");
		text += ")\n{";
		text += sourceCode;
		text += "}\n";
		text = text.Replace("lowpf", "float");
		return text.Replace("half", "float");
	}

	private string createFinalSourceMobile(ShaderLinkedSource linkedSrc)
	{
		string text = "";
		string input = linkedSrc.SourceCode;
		text += "#define RETURN_COLOR( Color ) Color\n";
		List<ShaderVariable> list = linkedSrc.VariableList.FindAll((ShaderVariable var) => var.VarType == ShaderVariable.VariableType.Variable_Uniform).ToList();
		list.Sort();
		foreach (ShaderVariable item in list)
		{
			if (item.SamplerType)
			{
				for (uint num = 0u; num < item.ArraySize; num++)
				{
					string text2 = text;
					text = text2 + "UNIFORM_SAMPLER2D( " + item.ID + ((item.ArraySize > 1) ? num.ToString() : "") + ", TEXUNIT" + (item.BaseRegister + num) + ");\n";
					input = Regex.Replace(input, item.ID + "\\[\\s*" + num + "\\s*\\]", item.ID + num);
				}
			}
			else
			{
				string text3 = text;
				text = text3 + "UNIFORM( " + item.Type + ", " + item.ID + ((item.ArraySize > 1) ? ("[" + item.ArraySize + "]") : "") + ");\n";
			}
		}
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
		foreach (ShaderVariable item2 in linkedSrc.VariableList.FindAll((ShaderVariable var) => var.VarType == inType || var.VarType == outType))
		{
			string text4 = item2.Semantic;
			string type = item2.Type;
			if (item2.Semantic.StartsWith("FACTOR") || item2.Semantic.StartsWith("INSTANCE"))
			{
				text4 = "COLOR1";
			}
			if (item2.VarType == ShaderVariable.VariableType.Variable_Varying && item2.Semantic.StartsWith("POSITION"))
			{
				string text5 = text;
				text = text5 + "OUT_BUILTIN(" + type + ", " + item2.ID + ", " + text4 + ")\n";
			}
			else
			{
				if (item2.VarType == ShaderVariable.VariableType.Variable_FragOut)
				{
					continue;
				}
				Match match = Regex.Match(type, "^(half|lowp)");
				string text6 = "DEFAULT";
				if (match.Captures.Count >= 1)
				{
					switch (match.Captures[0].Value)
					{
					case "half":
						text6 = "MEDIUM";
						break;
					case "lowp":
						text6 = "LOW";
						break;
					}
				}
				switch (linkedSrc.Pipeline.Type)
				{
				default:
					switch (item2.VarType)
					{
					case ShaderVariable.VariableType.Variable_Attribute:
						text += "ATTRIBUTE(";
						break;
					case ShaderVariable.VariableType.Variable_Varying:
						text = text + "OUT_VARYING_" + text6 + "(";
						break;
					}
					break;
				case ShaderPipeline.PipelineType.Fragment:
				{
					ShaderVariable.VariableType varType = item2.VarType;
					if (varType == ShaderVariable.VariableType.Variable_Varying)
					{
						text = text + "IN_VARYING_" + text6 + "(";
					}
					break;
				}
				}
				string text7 = text;
				text = text7 + type + ", " + item2.ID + ((item2.ArraySize > 1) ? ("[" + item2.ArraySize + "]") : "") + ", " + text4 + ");\n";
			}
		}
		ShaderVariable shaderVariable = null;
		if (linkedSrc.Pipeline.Type == ShaderPipeline.PipelineType.Fragment)
		{
			shaderVariable = linkedSrc.VariableList.FindLast((ShaderVariable v) => v.VarType == ShaderVariable.VariableType.Variable_FragOut);
			text += "OUT_BUILTIN(gl_FragColor, vec4, COLOR0)\n";
		}
		text = linkedSrc.Pipeline.Type switch
		{
			ShaderPipeline.PipelineType.Vertex => text + "void main ()", 
			ShaderPipeline.PipelineType.Fragment => text + "PIXEL_MAIN", 
			_ => throw new Exception("UE3 GFx does not support pipeline: " + linkedSrc.Pipeline.Type), 
		};
		input = Regex.Replace(input, "\\[([^\\]]+)\\]", "[int($1)]");
		input = Regex.Replace(input, "([^\\w\\.])(\\d+)([^\\w\\.])", "$1$2.0$3");
		text += "\n{\n";
		if (shaderVariable != null)
		{
			string text8 = text;
			text = text8 + shaderVariable.Type + " " + shaderVariable.ID + ";\n";
		}
		text += input;
		if (shaderVariable != null)
		{
			text = text + "gl_FragColor = " + shaderVariable.ID + ";\n";
		}
		text += "}\n";
		text = Regex.Replace(text, "(^|\\b)half([1-4])x([1-4])\\b", "mat$3");
		text = Regex.Replace(text, "(^|\\b)lowpf([1-4])x([1-4])", "mat$3");
		text = Regex.Replace(text, "(^|\\b)half([1-4])\\b", "vec$2");
		text = Regex.Replace(text, "(^|\\b)lowpf([1-4])\\b", "vec$2");
		text = Regex.Replace(text, "(^|\\b)half\\b", "float");
		text = Regex.Replace(text, "(^|\\b)lowpf\\b", "float");
		text = Regex.Replace(text, "(^|\\b)tex2Dlod\\s*\\((?'P0'" + ShaderVersion.SubexprRegex + "),(?'P1'" + ShaderVersion.SubexprRegex + "),(?'P2'" + ShaderVersion.SubexprRegex + ")\\)", "texture2D(${P0}, ${P1})");
		text = Regex.Replace(text, "(^|\\b)lerp\\b", "mix");
		text = Regex.Replace(text, "(^|\\b)(?:float|half|lowpf)([1-4])x([1-4])\\b", "mat$3");
		text = Regex.Replace(text, "(^|\\b)(?:float|half|lowpf)([1-4])\\b", "vec$2");
		text = Regex.Replace(text, "(^|\\b)(?:float|half|lowpf)\\b", "float");
		text = Regex.Replace(text, "(^|\\b)([0-9\\.]+)f", "$2");
		text = Regex.Replace(text, "[+-]0\\.?0*?f?([^\\.\\d])", "$2");
		text = Regex.Replace(text, "(^|\\b)tex2D\\b", "texture2D");
		text = Regex.Replace(text, "(^|\\b)tex2Dlod\\b", "texture2DLod");
		text = Regex.Replace(text, "(^|\\b)frac\\b", "fract");
		text = Regex.Replace(text, "mul\\s*\\((?'P0'" + ShaderVersion.SubexprRegex + "),(?'P1'" + ShaderVersion.SubexprRegex + ")\\)", "(${P0}) * (${P1})");
		foreach (ShaderVariable item3 in linkedSrc.VariableList.FindAll((ShaderVariable v) => v.VarType == ShaderVariable.VariableType.Variable_Varying && v.Semantic.StartsWith("POSITION")))
		{
			text = Regex.Replace(text, "(^|\\b)" + item3.ID + "\\b", "gl_Position");
		}
		return text;
	}

	public override string GetShaderFilename(ShaderLinkedSource src)
	{
		return "GFx_" + src.ID + SourceExtension;
	}

	public override string GetShaderDuplicateFilename(ShaderLinkedSource src)
	{
		return "GFx_" + src.SourceCodeDuplicateID + SourceExtension;
	}

	public override string GetSourceCodeContent(ShaderLinkedSource src)
	{
		if (GUsingMobileRHI)
		{
			return Regex.Replace(src.SourceCode, "^.*--MSF--(.*)--MSF--.*$", "$1", RegexOptions.Singleline);
		}
		return Regex.Replace(src.SourceCode, "^.*--USF--(.*)--USF--.*$", "$1", RegexOptions.Singleline);
	}
}
