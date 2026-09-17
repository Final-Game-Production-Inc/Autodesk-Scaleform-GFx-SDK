using System;
using System.Collections.Generic;
using System.Linq;
using System.Text.RegularExpressions;

namespace GFxShaderMaker.Platforms;

public abstract class ShaderVersion_GLSLCommon : ShaderVersion
{
	protected abstract string InstanceIDName { get; }

	public override bool RequireShaderCombos => true;

	public virtual bool UsesUniformBufferObjects => false;

	protected abstract string GLSLVersionString { get; }

	public ShaderVersion_GLSLCommon(ShaderPlatform platform, string id)
		: base(platform, id)
	{
	}

	protected abstract string GetGLSLExtensionStrings(ShaderLinkedSource linkedSrc);

	public override void PostLink_Instanced(ShaderLinkedSource linkedSrc)
	{
		base.PostLink_Batch(linkedSrc);
		ShaderVariable shaderVariable = linkedSrc.VariableList.Find((ShaderVariable var) => var.ID == "vbatch");
		ShaderVariable shaderVariable2 = linkedSrc.VariableList.Find((ShaderVariable var) => var.VarType == ShaderVariable.VariableType.Variable_Attribute && var.Semantic.StartsWith("factor", StringComparison.InvariantCultureIgnoreCase));
		if (shaderVariable2 != null && shaderVariable == null)
		{
			linkedSrc.SourceCode = Regex.Replace(linkedSrc.SourceCode, "\\b" + shaderVariable2.ID + ".b\\*255.01f", "float(" + InstanceIDName + ")");
		}
		else if (shaderVariable != null)
		{
			linkedSrc.SourceCode = Regex.Replace(linkedSrc.SourceCode, "\\b" + shaderVariable.ID + "\\b", "float(" + InstanceIDName + ")");
			linkedSrc.VariableList.Remove(shaderVariable);
		}
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

	public override string CreateFinalSource(ShaderLinkedSource linkedSrc)
	{
		string text = "";
		text += GLSLVersionString;
		text += GetGLSLExtensionStrings(linkedSrc);
		text += GetGLSLPrecisionString(linkedSrc);
		List<ShaderVariable> list = linkedSrc.VariableList.FindAll((ShaderVariable var) => var.VarType != ShaderVariable.VariableType.Variable_VirtualUniform && (linkedSrc.Pipeline.Type != ShaderPipeline.PipelineType.Vertex || !var.Semantic.StartsWith("POSITION") || var.VarType == ShaderVariable.VariableType.Variable_Attribute)).ToList();
		list.Sort();
		text = AddShaderUniforms(linkedSrc, text, list);
		text += "void main() { \n";
		string sourceCode = linkedSrc.SourceCode;
		PerformVersionSpecificReplacementSourceOnly(ref sourceCode, linkedSrc);
		text = text + sourceCode + "}\n";
		foreach (ShaderVariable item in linkedSrc.VariableList.FindAll((ShaderVariable v) => v.VarType == ShaderVariable.VariableType.Variable_Varying && v.Semantic.StartsWith("POSITION")))
		{
			text = Regex.Replace(text, "\\b" + item.ID + "\\b", "gl_Position");
		}
		PerformVersionSpecificReplacements(ref text, linkedSrc);
		return text;
	}

	protected virtual string AddShaderUniforms(ShaderLinkedSource linkedSrc, string shaderCode, List<ShaderVariable> uniforms)
	{
		foreach (ShaderVariable uniform in uniforms)
		{
			string shaderVariableQualifier = GetShaderVariableQualifier(uniform.VarType, linkedSrc.Pipeline);
			if (shaderVariableQualifier != null)
			{
				string text = shaderCode;
				shaderCode = text + shaderVariableQualifier + " " + uniform.Type + " " + uniform.ID + ((uniform.ArraySize > 1) ? ("[" + uniform.ArraySize + "]") : "") + ";\n";
			}
		}
		return shaderCode;
	}

	protected virtual void PerformVersionSpecificReplacements(ref string shaderCode, ShaderLinkedSource linkedSrc)
	{
		shaderCode = Regex.Replace(shaderCode, "\\b([0-9\\.]+)f", "$1");
		shaderCode = Regex.Replace(shaderCode, "[+-]0\\.?0*?f?([^\\.\\d])", "$1");
		shaderCode = Regex.Replace(shaderCode, "\\bfrac\\b", "fract");
		shaderCode = Regex.Replace(shaderCode, "\\bddx\\b", "dFdx");
		shaderCode = Regex.Replace(shaderCode, "\\bddy\\b", "dFdy");
		shaderCode = Regex.Replace(shaderCode, "\\btex2D\\b", "texture2D", RegexOptions.IgnoreCase);
		shaderCode = Regex.Replace(shaderCode, "\\btex2Dlod\\b", "texture2DLod", RegexOptions.IgnoreCase);
		shaderCode = Regex.Replace(shaderCode, "mul\\s*\\((?'P0'" + ShaderVersion.SubexprRegex + "),(?'P1'" + ShaderVersion.SubexprRegex + ")\\)", "(${P0}) * (${P1})");
	}

	protected virtual void PerformVersionSpecificReplacementSourceOnly(ref string sourceCode, ShaderLinkedSource linkedSrc)
	{
		sourceCode = Regex.Replace(sourceCode, "\\[([^\\]]+)\\]", "[int($1)]");
		sourceCode = Regex.Replace(sourceCode, "([^\\w\\.])(\\d+)([^\\w\\.])", "$1$2.0$3");
	}

	protected abstract string GetShaderVariableQualifier(ShaderVariable.VariableType VarType, ShaderPipeline pipeline);

	protected virtual string GetGLSLPrecisionString(ShaderLinkedSource linekdSrc)
	{
		return "";
	}
}
