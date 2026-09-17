using System;

namespace GFxShaderMaker;

public class ShaderPipeline : IEquatable<ShaderPipeline>
{
	public enum PipelineType
	{
		Vertex,
		Fragment,
		Geometry,
		Hull,
		Domain,
		Compute
	}

	public PipelineType Type { get; private set; }

	public string Name => GetPipelineName(Type);

	public char Letter => GetPipelineLetter(Type);

	public static string GetPipelineName(PipelineType type)
	{
		return type switch
		{
			PipelineType.Fragment => "Frag", 
			PipelineType.Geometry => "Geometry", 
			PipelineType.Hull => "Hull", 
			PipelineType.Domain => "Domain", 
			PipelineType.Compute => "Compute", 
			_ => "Vertex", 
		};
	}

	public static char GetPipelineLetter(PipelineType type)
	{
		return GetPipelineName(type)[0];
	}

	public ShaderPipeline(PipelineType type)
	{
		Type = type;
	}

	public static bool operator ==(ShaderPipeline p0, ShaderPipeline p1)
	{
		if ((object)p0 == null && (object)p1 == null)
		{
			return true;
		}
		if ((object)p0 == null || (object)p1 == null)
		{
			return false;
		}
		return p0.Type == p1.Type;
	}

	public static bool operator !=(ShaderPipeline p0, ShaderPipeline p1)
	{
		return !(p0 == p1);
	}

	public bool Equals(ShaderPipeline other)
	{
		return this == other;
	}

	public override bool Equals(object obj)
	{
		ShaderPipeline shaderPipeline = obj as ShaderPipeline;
		if (shaderPipeline == null)
		{
			return false;
		}
		return this == shaderPipeline;
	}

	public override int GetHashCode()
	{
		return Convert.ToInt32(Type);
	}
}
