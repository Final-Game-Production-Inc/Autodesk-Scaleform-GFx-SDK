using System.Collections.Generic;

namespace GFxShaderMaker;

public class ShaderVariableCompareIDsSizeAndType : IEqualityComparer<ShaderVariable>
{
	public bool Equals(ShaderVariable x, ShaderVariable y)
	{
		if (x.ID == y.ID && x.Type.CompareTo(y.Type) == 0)
		{
			return x.Size == y.Size;
		}
		return false;
	}

	public int GetHashCode(ShaderVariable obj)
	{
		return obj.ID.GetHashCode() ^ obj.Type.GetHashCode() ^ (int)obj.Size;
	}
}
