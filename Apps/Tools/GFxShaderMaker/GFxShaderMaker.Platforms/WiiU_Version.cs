using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text.RegularExpressions;

namespace GFxShaderMaker.Platforms;

public class WiiU_Version : ShaderVersion
{
	private Dictionary<string, uint> SemanticLayout = new Dictionary<string, uint>();

	private uint NextLayoutIndex;

	public override string SourceExtension => ".glsl";

	protected override string BatchIndexType => "int";

	public WiiU_Version(ShaderPlatform platform)
		: base(platform, "WiiU")
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
		List<ShaderVariable> list = linkedSrc.VariableList.FindAll((ShaderVariable var) => var.VarType != ShaderVariable.VariableType.Variable_VirtualUniform && var.VarType != ShaderVariable.VariableType.Variable_FragOut).ToList();
		list.Sort();
		foreach (ShaderVariable item in list)
		{
			if ((linkedSrc.Pipeline.Type == ShaderPipeline.PipelineType.Vertex && item.VarType == ShaderVariable.VariableType.Variable_Varying) || (linkedSrc.Pipeline.Type == ShaderPipeline.PipelineType.Fragment && item.VarType == ShaderVariable.VariableType.Variable_Varying))
			{
				text = ((!item.Semantic.StartsWith("POSITION")) ? (text + string.Format("layout(location = {0}) {1} {2} {3}{4};\n", SemanticLayout[item.Semantic], (linkedSrc.Pipeline.Type == ShaderPipeline.PipelineType.Vertex) ? "out" : "in", item.Type, item.ID, (item.ArraySize > 1) ? ("[" + item.ArraySize + "]") : "")) : (text + $"out gl_PerVertex {{ layout(location = {SemanticLayout[item.Semantic]}) {item.Type} {item.ID}; }};\n"));
			}
			else
			{
				string text2 = item.VarType.ToString().Split("_".ToCharArray(), StringSplitOptions.RemoveEmptyEntries)[1].ToLower();
				string text3 = text;
				text = text3 + text2 + " " + item.Type + " " + item.ID + ((item.ArraySize > 1) ? ("[" + item.ArraySize + "]") : "") + ";\n";
			}
		}
		if (linkedSrc.Pipeline.Type == ShaderPipeline.PipelineType.Fragment)
		{
			text += string.Format("in gl_PerVertex {{ layout(location = {0}) float4 gl_Position; }};\n", SemanticLayout["POSITION0"]);
		}
		text += "void main() { \n";
		string sourceCode = linkedSrc.SourceCode;
		sourceCode = Regex.Replace(sourceCode, "\\[([^\\]]+)\\]", "[int($1)]");
		sourceCode = Regex.Replace(sourceCode, "([^\\w\\.])(\\d+)([^\\w\\.])", "$1$2.0$3");
		text = text + sourceCode + "}\n";
		text = Regex.Replace(text, "(^|\\b)lerp\\b", "mix");
		text = Regex.Replace(text, "(^|\\b)(?:float|half|lowpf)([1-4])x([1-4])\\b", "mat$3");
		text = Regex.Replace(text, "(^|\\b)(?:float|half|lowpf)([1-4])\\b", "vec$2");
		text = Regex.Replace(text, "(^|\\b)(?:float|half|lowpf)\\b", "float");
		text = Regex.Replace(text, "(^|\\b)([0-9\\.]+)f", "$2");
		text = Regex.Replace(text, "[+-]0\\.?0*?f?([^\\.\\d])", "$2");
		text = Regex.Replace(text, "(^|\\b)tex2D\\b", "texture2D");
		text = Regex.Replace(text, "(^|\\b)tex2Dlod\\b", "texture2DLod");
		text = Regex.Replace(text, "(^|\\b)frac\\b", "fract");
		text = Regex.Replace(text, "\\bddx\\b", "dFdx");
		text = Regex.Replace(text, "\\bddy\\b", "dFdy");
		text = Regex.Replace(text, "mul\\s*\\((?'P0'" + ShaderVersion.SubexprRegex + "),(?'P1'" + ShaderVersion.SubexprRegex + ")\\)", "(${P0}) * (${P1})");
		foreach (ShaderVariable item2 in linkedSrc.VariableList.FindAll((ShaderVariable v) => v.VarType == ShaderVariable.VariableType.Variable_FragOut))
		{
			text = Regex.Replace(text, "\\b" + item2.ID + "\\b", "gl_FragColor");
		}
		foreach (ShaderVariable item3 in linkedSrc.VariableList.FindAll((ShaderVariable v) => v.VarType == ShaderVariable.VariableType.Variable_Varying && v.Semantic.StartsWith("POSITION")))
		{
			text = Regex.Replace(text, "\\b" + item3.ID + "\\b", "gl_Position");
		}
		return text;
	}

	public override void AssignRegisters(ShaderLinkedSource src)
	{
		base.AssignRegisters(src);
		foreach (ShaderVariable variable in src.VariableList)
		{
			if (!SemanticLayout.TryGetValue(variable.Semantic, out var _))
			{
				SemanticLayout[variable.Semantic] = NextLayoutIndex++;
			}
		}
	}

	public override void WriteBinaryShaderSource(StreamWriter sourceFile)
	{
		foreach (ShaderLinkedSource value2 in LinkedSourceDuplicates.Values)
		{
			string shaderOutputFilename = GetShaderOutputFilename(value2);
			StreamReader streamReader = File.OpenText(shaderOutputFilename);
			string value = streamReader.ReadToEnd().Replace("static GX2", "GX2");
			sourceFile.Write(value);
			streamReader.Close();
		}
	}

	public string GetShaderOutputFilename(ShaderLinkedSource src)
	{
		string path = Path.Combine(base.SourceDirectory, GetShaderFilename(src));
		return Path.Combine(Path.GetDirectoryName(path), Path.GetFileNameWithoutExtension(path) + ".h");
	}
}
