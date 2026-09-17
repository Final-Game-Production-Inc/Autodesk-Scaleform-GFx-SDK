using System;
using System.Collections.Generic;
using System.Linq;
using System.Text.RegularExpressions;

namespace GFxShaderMaker.Platforms;

public class ShaderVersion_SM40 : ShaderVersion_D3DCommon
{
	public ShaderVersion_SM40(ShaderPlatform platform, string id)
		: base(platform, id)
	{
		SupportedPipelines.Add(new ShaderPipeline(ShaderPipeline.PipelineType.Geometry));
	}

	public override string GetShaderProfile(ShaderPipeline pipeline)
	{
		return pipeline.Type switch
		{
			ShaderPipeline.PipelineType.Fragment => "ps_4_0", 
			ShaderPipeline.PipelineType.Vertex => "vs_4_0", 
			ShaderPipeline.PipelineType.Geometry => "gs_4_0", 
			_ => null, 
		};
	}

	public override void PostLink_Batch(ShaderLinkedSource linkedSrc)
	{
		base.PostLink_Batch(linkedSrc);
		ShaderVariable shaderVariable = linkedSrc.VariableList.Find((ShaderVariable var) => var.ID == "factor");
		ShaderVariable shaderVariable2 = linkedSrc.VariableList.Find((ShaderVariable var) => var.ID == "vbatch");
		if (shaderVariable != null && shaderVariable2 != null)
		{
			linkedSrc.VariableList.Remove(shaderVariable2);
			linkedSrc.SourceCode = linkedSrc.SourceCode.Replace("vbatch", "afactor.b");
		}
	}

	public override string CreateFinalSource(ShaderLinkedSource linkedSrc)
	{
		string shaderCode = RootSignature;
		List<ShaderVariable> list = linkedSrc.VariableList.FindAll((ShaderVariable shaderVariable) => shaderVariable.VarType == ShaderVariable.VariableType.Variable_Uniform && !shaderVariable.SamplerType).ToList();
		List<ShaderVariable> list2 = linkedSrc.VariableList.FindAll((ShaderVariable shaderVariable) => shaderVariable.VarType == ShaderVariable.VariableType.Variable_Uniform && shaderVariable.SamplerType).ToList();
		list.Sort();
		writeSourceUniforms(ref shaderCode, linkedSrc, list);
		int num = 0;
		foreach (ShaderVariable item in list2)
		{
			object obj = shaderCode;
			shaderCode = string.Concat(obj, "SamplerState sampler_", item.ID, (item.ArraySize > 1) ? ("[" + item.ArraySize + "]") : "", " : register(s", num++, ");\n");
			object obj2 = shaderCode;
			shaderCode = string.Concat(obj2, Regex.Replace(item.Type, "^sampler", "Texture"), " ", item.ID, (item.ArraySize > 1) ? ("[" + item.ArraySize + "]") : "", " : register(t", item.BaseRegister, ");\n");
		}
		shaderCode += RootSignatureAttribute;
		shaderCode += "void main( ";
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
				shaderCode += ",\n           ";
			}
			flag = false;
			string text = var.Semantic;
			string text2 = var.Type;
			if (var.Semantic.StartsWith("FACTOR") || var.Semantic.StartsWith("INSTANCE"))
			{
				if (var.Semantic.StartsWith("INSTANCE") && linkedSrc.PostFunctions.Find((string s) => string.Compare(s, "Instanced", ignoreCase: true) == 0) != null)
				{
					text = "SV_InstanceID";
				}
				else
				{
					text = "COLOR";
					List<ShaderVariable> list3 = linkedSrc.VariableList.FindAll((ShaderVariable v) => v.Semantic.StartsWith("COLOR") && v.VarType == var.VarType);
					string value = "-1";
					if (list3.Count > 0)
					{
						value = Regex.Replace(list3.Max((ShaderVariable v) => v.Semantic), "^.*(\\d+)$", "$1");
					}
					text += Convert.ToInt32(value) + (var.Semantic.StartsWith("FACTOR") ? 1 : 2);
				}
			}
			else if (linkedSrc.Pipeline.Type == ShaderPipeline.PipelineType.Fragment && var.VarType == outType && var.Semantic.StartsWith("COLOR"))
			{
				text = Regex.Replace(var.Semantic, "^COLOR", "SV_Target");
			}
			else if (var.Semantic.StartsWith("POSITION"))
			{
				text = Regex.Replace(var.Semantic, "^POSITION", "SV_Position");
				text = Regex.Replace(text, "0$", "");
			}
			if (var.Semantic.StartsWith("INSTANCE") || text.StartsWith("SV_InstanceID"))
			{
				text2 = Regex.Replace(text2, "float", "uint");
				text2 = Regex.Replace(text2, "lowpf", "uint");
			}
			string text3 = shaderCode;
			shaderCode = text3 + ((var.VarType == outType) ? "out " : "") + text2 + " " + var.ID + ((var.ArraySize > 1) ? ("[" + var.ArraySize + "]") : "") + " : " + text;
		}
		shaderCode += ")\n{";
		shaderCode = shaderCode + linkedSrc.SourceCode + "}\n";
		shaderCode = shaderCode.Replace("lowpf", "float");
		shaderCode = Regex.Replace(shaderCode, "tex\\dD\\s*\\(\\s*([^,]+)", "$1.Sample(sampler_$1");
		return Regex.Replace(shaderCode, "tex\\dDlod\\s*\\(\\s*([^,]+)", "$1.SampleLevel(sampler_$1");
	}

	protected virtual void writeSourceUniforms(ref string shaderCode, ShaderLinkedSource src, List<ShaderVariable> uniforms)
	{
		if (uniforms.Count <= 0)
		{
			return;
		}
		shaderCode += "cbuffer Constants { \n";
		foreach (ShaderVariable uniform in uniforms)
		{
			object obj = shaderCode;
			shaderCode = string.Concat(obj, uniform.Type, " ", uniform.ID, (uniform.ArraySize > 1) ? ("[" + uniform.ArraySize + "]") : "", " : packoffset(c", uniform.BaseRegister, ");\n");
		}
		shaderCode += "};\n\n";
	}
}
