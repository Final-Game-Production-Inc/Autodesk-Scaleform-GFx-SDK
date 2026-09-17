using System;
using System.Collections.Generic;
using System.Linq;
using System.Text.RegularExpressions;

namespace GFxShaderMaker.Platforms;

public class ShaderVersion_Metal : ShaderVersion
{
	public override string SourceExtension => ".metal";

	public ShaderVersion_Metal(ShaderPlatform platform)
		: base(platform, "Metal")
	{
	}

	public override string GetVariableUniformRegisterType(ShaderVariable var)
	{
		if (!var.SamplerType)
		{
			return "uniform";
		}
		return "texture";
	}

	public override void PostLink_Instanced(ShaderLinkedSource linkedSrc)
	{
	}

	public override string CreateFinalSource(ShaderLinkedSource linkedSrc)
	{
		string text = "#include <metal_stdlib>\nusing namespace metal;\n\n";
		string text2 = "";
		string text3 = "";
		string text4 = "";
		string input = linkedSrc.SourceCode;
		bool flag = false;
		List<ShaderVariable> list = linkedSrc.VariableList.FindAll((ShaderVariable var) => var.VarType == ShaderVariable.VariableType.Variable_Uniform).ToList();
		list.Sort();
		foreach (ShaderVariable item in list)
		{
			if (item.ArraySize <= 1 || linkedSrc.Pipeline.Type != ShaderPipeline.PipelineType.Fragment)
			{
				continue;
			}
			for (uint num = 0u; num < item.ArraySize; num++)
			{
				input = Regex.Replace(input, item.ID + "\\[\\s*" + num + "\\s*\\]", item.ID + ((num != 0) ? num.ToString() : ""));
				if (!item.SamplerType)
				{
					throw new Exception("Metal cannot handle non-sampler uniform arrays, shader must be refactored. VariableName = " + item.ID + " in shader " + linkedSrc.ID);
				}
			}
		}
		for (uint num2 = 0u; num2 < 4; num2++)
		{
			ShaderVariable.VariableType variableType = ShaderVariable.VariableType.Variable_Attribute;
			string text5 = "";
			bool flag2 = false;
			switch (linkedSrc.Pipeline.Type)
			{
			case ShaderPipeline.PipelineType.Vertex:
				switch (num2)
				{
				case 0u:
					text2 += "struct VertexInput {\n";
					variableType = ShaderVariable.VariableType.Variable_Attribute;
					if (!flag)
					{
						text5 = "[[attribute({0})]]";
					}
					break;
				case 1u:
					text2 += "struct VertexOutput {\n";
					variableType = ShaderVariable.VariableType.Variable_Varying;
					break;
				case 2u:
					text2 += "struct Uniforms {\n";
					variableType = ShaderVariable.VariableType.Variable_Uniform;
					break;
				case 3u:
					variableType = ShaderVariable.VariableType.Variable_Uniform;
					text5 = "[[texture({0})]]";
					flag2 = true;
					break;
				}
				break;
			case ShaderPipeline.PipelineType.Fragment:
				switch (num2)
				{
				case 0u:
					text2 += "struct VertexOutput {\n";
					variableType = ShaderVariable.VariableType.Variable_Varying;
					break;
				case 1u:
					text2 += "struct FragmentOutput {\n";
					variableType = ShaderVariable.VariableType.Variable_FragOut;
					text5 = "[[color({0})]]";
					break;
				case 2u:
					text2 += "struct Uniforms {\n";
					variableType = ShaderVariable.VariableType.Variable_Uniform;
					break;
				case 3u:
					variableType = ShaderVariable.VariableType.Variable_Uniform;
					text5 = "[[texture({0})]]";
					flag2 = true;
					break;
				}
				break;
			}
			uint num3 = 0u;
			foreach (ShaderVariable variable in linkedSrc.VariableList)
			{
				if (variable.VarType != variableType || variable.SamplerType != flag2)
				{
					continue;
				}
				if (variable.Type == "sampler2D")
				{
					variable.Type = "texture2d<float>";
				}
				if (variable.VarType == ShaderVariable.VariableType.Variable_Varying && variable.Semantic.StartsWith("POSITION") && text5 == "")
				{
					text5 = "[[position]]";
				}
				string text6 = string.Format(text5, num3++);
				if (num2 != 3)
				{
					string text7 = text2;
					text2 = text7 + variable.Type + " " + variable.ID + ((variable.ArraySize > 1) ? ("[" + variable.ArraySize + "]") : "") + " " + text6 + ";\n";
				}
				else
				{
					for (uint num4 = 0u; num4 < variable.ArraySize; num4++)
					{
						string text8 = text3;
						text3 = text8 + ",\n" + variable.Type + " " + variable.ID + ((num4 != 0) ? num4.ToString() : "") + " " + text6;
						string text9 = Regex.Replace(text6, "texture", "sampler");
						string text10 = text3;
						text3 = text10 + ",\nsampler sampler_" + variable.ID + ((num4 != 0) ? num4.ToString() : "") + " " + text9;
						if (num4 < variable.ArraySize - 1)
						{
							text6 = string.Format(text5, num3++);
						}
					}
				}
				switch (num2)
				{
				case 0u:
					input = Regex.Replace(input, "\\b" + variable.ID + "\\b", "input." + variable.ID);
					break;
				case 1u:
					input = Regex.Replace(input, "\\b" + variable.ID + "\\b", "output." + variable.ID);
					break;
				case 2u:
					input = Regex.Replace(input, "\\b" + variable.ID + "\\b", "uniforms." + variable.ID);
					break;
				}
			}
			if (num2 != 3)
			{
				text2 += "};\n";
			}
		}
		switch (linkedSrc.Pipeline.Type)
		{
		case ShaderPipeline.PipelineType.Vertex:
			if (flag)
			{
				text4 += "vertex VertexOutput\n";
				text4 += "shaderentrypoint(device VertexInput*   vinput       [[buffer(0)]],\n";
				text4 += "                 constant Uniforms&    uniforms     [[buffer(1)]],\n";
				text4 += "                 unsigned int          vid          [[vertex_id]])\n";
				text4 += "{\n";
				text4 += "    device VertexInput& input = vinput[vid];\n";
				text4 += "    VertexOutput output;\n";
			}
			else
			{
				text4 += "vertex VertexOutput\n";
				text4 += "shaderentrypoint(VertexInput           input        [[stage_in]],\n";
				text4 += "                 constant Uniforms&    uniforms     [[buffer(1)]])\n";
				text4 += "{\n";
				text4 += "    VertexOutput output;\n";
			}
			break;
		case ShaderPipeline.PipelineType.Fragment:
			text4 += "fragment FragmentOutput\n";
			text4 += "shaderentrypoint(const VertexOutput input [[stage_in]],\n";
			text4 = text4 + "      constant Uniforms& uniforms [[buffer(1)]]" + text3 + ")\n";
			text4 += "{\n";
			text4 += "    FragmentOutput output;\n";
			break;
		}
		input = Regex.Replace(input, "\\[([^\\]]+)\\]", "[int($1)]");
		input = Regex.Replace(input, "\\blerp\\b", "mix");
		input = Regex.Replace(input, "\\bfrac\\b", "fract");
		input = Regex.Replace(input, "\\bddx\\b", "dfdx");
		input = Regex.Replace(input, "\\bddy\\b", "dfdy");
		input = Regex.Replace(input, "mul\\s*\\((?'P0'" + ShaderVersion.SubexprRegex + "),(?'P1'" + ShaderVersion.SubexprRegex + ")\\)", "(${P0}) * (${P1})");
		input = Regex.Replace(input, "tex\\dD\\s*\\(\\s*([^\\[,]+)\\[([^\\]]+)\\],(?'P0'" + ShaderVersion.SubexprRegex + ")", "$1.sample(sampler_$1, ${P0}, $2");
		input = Regex.Replace(input, "tex\\dDlod\\s*\\(\\s*([^\\[,]+)\\[([^\\]]+)\\],(?'P0'" + ShaderVersion.SubexprRegex + ")", "$1.sample(sampler_$1, ${P0}, $2");
		input = Regex.Replace(input, "tex\\dD\\s*\\(\\s*([^,]+)", "$1.sample(sampler_$1");
		input = Regex.Replace(input, "tex\\dDlod\\s*\\(\\s*([^,]+)", "$1.sample(sampler_$1");
		input = Regex.Replace(input, "\\bdiscard\\b", "discard_fragment()");
		string text11 = "    return output;\n}\n";
		string input2 = text + text2 + text4 + input + text11;
		input2 = Regex.Replace(input2, "lowpf", "float");
		return Regex.Replace(input2, "half", "float");
	}

	public override string GetSourceCodeContent(ShaderLinkedSource src)
	{
		string sourceCode = src.SourceCode;
		return Regex.Replace(sourceCode, "shaderentrypoint", src.ID);
	}
}
