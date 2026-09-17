using System;
using System.Text.RegularExpressions;

namespace GFxShaderMaker;

public class ShaderVariable : IEquatable<ShaderVariable>, IComparable<ShaderVariable>, ICloneable
{
	public enum VariableType
	{
		Variable_Uniform,
		Variable_Attribute,
		Variable_Varying,
		Variable_FragOut,
		Variable_VirtualUniform,
		Variable_VirtualAttribute
	}

	public string ID;

	public string Semantic;

	public VariableType VarType;

	public string Type;

	public uint ArraySize = 1u;

	public object ExtraData;

	public bool SamplerType;

	private bool BaseSet;

	private uint BaseReg;

	public uint RegisterCount
	{
		get
		{
			if (!SamplerType)
			{
				return RegisterCountPerElement * ArraySize;
			}
			return ArraySize;
		}
	}

	public uint RegisterCountPerElement
	{
		get
		{
			Match match = Regex.Match(Type, "(\\d)x?(\\d)?$");
			if (match.Groups[2].Length != 0)
			{
				return Convert.ToUInt32(match.Groups[2].Value);
			}
			return 1u;
		}
	}

	public uint BaseRegister
	{
		get
		{
			if (!BaseSet)
			{
				throw new Exception("Internal error - BaseRegister invalid access.");
			}
			return BaseReg;
		}
		set
		{
			BaseSet = true;
			BaseReg = value;
		}
	}

	public uint ElementCount
	{
		get
		{
			if (!SamplerType)
			{
				Match match = Regex.Match(Type, "(\\d)x?(\\d)?$");
				return ((match.Groups[1].Length == 0) ? 1 : Convert.ToUInt32(match.Groups[1].Value)) * ((match.Groups[2].Length == 0) ? 1 : Convert.ToUInt32(match.Groups[2].Value));
			}
			return 1u;
		}
	}

	public uint Size => ElementCount * ArraySize;

	public void ReadFromString(string vstr)
	{
		Match match = Regex.Match(vstr, "(uniform|attribute|varying|fragout)\\s+(\\w+)\\s+(\\w+)\\s*(\\s*\\[\\s*(\\d+)\\s*\\])?(\\s*\\:\\s*(\\w+))?");
		switch (match.Groups[1].Value)
		{
		default:
			VarType = VariableType.Variable_Uniform;
			break;
		case "attribute":
			VarType = VariableType.Variable_Attribute;
			break;
		case "varying":
			VarType = VariableType.Variable_Varying;
			break;
		case "fragout":
			VarType = VariableType.Variable_FragOut;
			break;
		}
		Type = match.Groups[2].Value;
		ID = match.Groups[3].Value;
		Semantic = match.Groups[7].Value;
		ArraySize = (string.IsNullOrEmpty(match.Groups[5].Value) ? 1u : Convert.ToUInt32(match.Groups[5].Value));
		if (Type.StartsWith("sampler", StringComparison.InvariantCultureIgnoreCase))
		{
			SamplerType = true;
		}
	}

	public bool Equals(ShaderVariable other)
	{
		if (ID == other.ID)
		{
			return VarType == other.VarType;
		}
		return false;
	}

	public int CompareTo(ShaderVariable other)
	{
		if (other == null)
		{
			return -1;
		}
		if (other.VarType != VarType)
		{
			if (VarType >= other.VarType)
			{
				return 1;
			}
			return -1;
		}
		return ID.CompareTo(other.ID);
	}

	public object Clone()
	{
		ShaderVariable shaderVariable = new ShaderVariable();
		shaderVariable.ID = ID;
		shaderVariable.Semantic = Semantic;
		shaderVariable.Type = Type;
		shaderVariable.VarType = VarType;
		shaderVariable.ExtraData = ExtraData;
		shaderVariable.BaseSet = BaseSet;
		shaderVariable.BaseReg = BaseReg;
		shaderVariable.ArraySize = ArraySize;
		shaderVariable.SamplerType = SamplerType;
		return shaderVariable;
	}
}
