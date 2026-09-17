using System.Collections.Generic;

namespace GFxShaderMaker;

public class ShaderVariableCompareIDs : IEqualityComparer<ShaderVariable>
{
	public bool Equals(ShaderVariable x, ShaderVariable y)
	{
		return x.ID == y.ID;
	}

	public int GetHashCode(ShaderVariable obj)
	{
		return obj.ID.GetHashCode();
	}
}
