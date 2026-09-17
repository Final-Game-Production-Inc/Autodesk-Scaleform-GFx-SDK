using System;
using System.Collections.Generic;
using System.Text.RegularExpressions;
using System.Xml;

namespace GFxShaderMaker;

public class ShaderSource : ShaderRestriction
{
	public string ID;

	public ShaderPipeline.PipelineType PipelineType;

	public string RawSource;

	public IList<ShaderVariable> Variables;

	public string CodeOnly;

	public override void ReadFromXml(XmlElement sourceNode)
	{
		ID = sourceNode.GetAttribute("id");
		switch (sourceNode.GetAttribute("pipeline"))
		{
		default:
			PipelineType = ShaderPipeline.PipelineType.Vertex;
			break;
		case "Fragment":
			PipelineType = ShaderPipeline.PipelineType.Fragment;
			break;
		case "Geometry":
			PipelineType = ShaderPipeline.PipelineType.Geometry;
			break;
		case "Hull":
			PipelineType = ShaderPipeline.PipelineType.Hull;
			break;
		case "Domain":
			PipelineType = ShaderPipeline.PipelineType.Domain;
			break;
		case "Compute":
			PipelineType = ShaderPipeline.PipelineType.Compute;
			break;
		}
		RawSource = sourceNode.InnerText;
		int num = RawSource.IndexOf('{');
		int num2 = RawSource.LastIndexOf('}');
		CodeOnly = RawSource.Substring(num + 1, RawSource.Length - num - 1 - (RawSource.Length - num2));
		Variables = new List<ShaderVariable>();
		Match match = Regex.Match(RawSource, "\\(([^\\)]*)\\)");
		string text = match.Captures[0].ToString();
		string[] array = text.Split("(),".ToCharArray(), StringSplitOptions.RemoveEmptyEntries);
		foreach (string text2 in array)
		{
			string text3 = text2.Trim();
			if (text3.Length != 0)
			{
				ShaderVariable shaderVariable = new ShaderVariable();
				shaderVariable.ReadFromString(text3);
				Variables.Add(shaderVariable);
			}
		}
		base.ReadFromXml(sourceNode);
	}
}
