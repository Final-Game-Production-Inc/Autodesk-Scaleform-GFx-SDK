using System.Collections.Generic;

namespace GFxShaderMaker;

public class ShaderPipelineCompareTypes : IEqualityComparer<ShaderPipeline>
{
	public bool Equals(ShaderPipeline x, ShaderPipeline y)
	{
		return x.Type == y.Type;
	}

	public int GetHashCode(ShaderPipeline obj)
	{
		return obj.Type.GetHashCode();
	}
}
