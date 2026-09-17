using System;
using System.Collections.Generic;
using System.Linq;
using System.Text.RegularExpressions;

namespace GFxShaderMaker.Platforms;

internal class ShaderVersion_PS4 : ShaderVersion
{
	public override string SourceExtension => ".pssl";

	public ShaderVersion_PS4(ShaderPlatform platform)
		: base(platform, "PS4")
	{
		SupportedPipelines.Add(new ShaderPipeline(ShaderPipeline.PipelineType.Hull));
		SupportedPipelines.Add(new ShaderPipeline(ShaderPipeline.PipelineType.Domain));
		SupportedPipelines.Add(new ShaderPipeline(ShaderPipeline.PipelineType.Geometry));
		SupportedPipelines.Add(new ShaderPipeline(ShaderPipeline.PipelineType.Compute));
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
		string text2 = linkedSrc.SourceCode;
		List<ShaderVariable> list = linkedSrc.VariableList.FindAll((ShaderVariable shaderVariable) => shaderVariable.VarType == ShaderVariable.VariableType.Variable_Uniform && !shaderVariable.SamplerType).ToList();
		List<ShaderVariable> list2 = linkedSrc.VariableList.FindAll((ShaderVariable shaderVariable) => shaderVariable.VarType == ShaderVariable.VariableType.Variable_Uniform && shaderVariable.SamplerType).ToList();
		list.Sort();
		if (list.Count > 0)
		{
			text += "ConstantBuffer Constants : register(b0) { \n";
			foreach (ShaderVariable item in list)
			{
				object obj = text;
				text = string.Concat(obj, item.Type, " ", item.ID, (item.ArraySize > 1) ? ("[" + item.ArraySize + "]") : "", " : packoffset(c", item.BaseRegister, ");\n");
			}
			text += "};\n\n";
		}
		if (list2.Count > 0)
		{
			text += "SamplerState sampler : register(s0);\n";
		}
		foreach (ShaderVariable item2 in list2)
		{
			object obj2 = text;
			text = string.Concat(obj2, Regex.Replace(item2.Type, "^sampler", "Texture"), " ", item2.ID, (item2.ArraySize > 1) ? ("[" + item2.ArraySize + "]") : "", " : register(t", item2.BaseRegister, ");\n");
		}
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
		for (uint num = 0u; num < 2; num++)
		{
			bool flag2 = num == 0;
			text = ((!flag2) ? (text + "struct ShaderOutputType\n{\n           ") : (text + "struct ShaderInputType\n{\n            "));
			flag = true;
			ShaderVariable var;
			foreach (ShaderVariable item3 in linkedSrc.VariableList.FindAll((ShaderVariable shaderVariable) => shaderVariable.VarType == inType || shaderVariable.VarType == outType))
			{
				var = item3;
				string text3 = var.Semantic;
				string text4 = var.Type;
				if (var.Semantic.StartsWith("FACTOR") || var.Semantic.StartsWith("INSTANCE"))
				{
					if (var.Semantic.StartsWith("INSTANCE") && linkedSrc.PostFunctions.Find((string s) => string.Compare(s, "Instanced", ignoreCase: true) == 0) != null)
					{
						text3 = "S_INSTANCE_ID";
					}
					else
					{
						text3 = "COLOR";
						List<ShaderVariable> list3 = linkedSrc.VariableList.FindAll((ShaderVariable v) => v.Semantic.StartsWith("COLOR") && v.VarType == var.VarType);
						string value = "-1";
						if (list3.Count > 0)
						{
							value = Regex.Replace(list3.Max((ShaderVariable v) => v.Semantic), "^.*(\\d+)$", "$1");
						}
						text3 += Convert.ToInt32(value) + (var.Semantic.StartsWith("FACTOR") ? 1 : 2);
					}
				}
				else if (linkedSrc.Pipeline.Type == ShaderPipeline.PipelineType.Fragment && var.VarType == outType && var.Semantic.StartsWith("COLOR"))
				{
					text3 = Regex.Replace(var.Semantic, "^COLOR", "S_TARGET_OUTPUT");
				}
				else if (var.Semantic.StartsWith("POSITION"))
				{
					if (linkedSrc.Pipeline.Type == ShaderPipeline.PipelineType.Vertex && var.VarType == outType)
					{
						text3 = Regex.Replace(var.Semantic, "^POSITION", "S_POSITION");
					}
					text3 = Regex.Replace(text3, "0$", "");
				}
				if (var.Semantic.StartsWith("INSTANCE") || text3.StartsWith("S_INSTANCE_ID"))
				{
					text4 = Regex.Replace(text4, "float", "uint");
					text4 = Regex.Replace(text4, "lowpf", "uint");
				}
				if ((flag2 && var.VarType != outType) || (!flag2 && var.VarType == outType))
				{
					if (!flag)
					{
						text += ";\n           ";
					}
					flag = false;
					string text5 = text;
					text = text5 + text4 + " " + var.ID + ((var.ArraySize > 1) ? ("[" + var.ArraySize + "]") : "") + " : " + text3 + ";";
					text2 = Regex.Replace(text2, "\\b" + var.ID + "\\b", (flag2 ? "shaderInput." : "shaderOutput.") + var.ID);
				}
			}
			string option = CommandLineParser.GetOption(CommandLineParser.Options.Config);
			if (flag2 && option.StartsWith("Debug"))
			{
				if (!flag)
				{
					text += ";\n           ";
				}
				switch (linkedSrc.Pipeline.Type)
				{
				case ShaderPipeline.PipelineType.Vertex:
					text += "uint DebugVertexID : S_VERTEX_ID;\n";
					break;
				case ShaderPipeline.PipelineType.Fragment:
					text += "float4 DebugPosition : S_POSITION;\n";
					break;
				case ShaderPipeline.PipelineType.Compute:
					text += "uint3 DebugCompute : S_DISPATCH_THREAD_ID;\n";
					break;
				}
			}
			text += "\n};\n";
		}
		text += "ShaderOutputType main( ShaderInputType shaderInput )\n{\n";
		text = text + "    ShaderOutputType shaderOutput;\n" + text2 + "    return shaderOutput;\n}\n";
		text = text.Replace("lowpf", "float");
		text = text.Replace("half", "float");
		text = Regex.Replace(text, "tex\\dD\\s*\\(\\s*([^,]+)", "$1.Sample(sampler");
		return Regex.Replace(text, "tex\\dDlod\\s*\\(\\s*([^,]+)", "$1.SampleLOD(sampler");
	}
}
