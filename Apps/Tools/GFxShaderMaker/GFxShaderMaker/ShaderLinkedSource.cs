using System;
using System.Collections.Generic;
using System.Linq;
using System.Text.RegularExpressions;

namespace GFxShaderMaker;

public class ShaderLinkedSource
{
	public string ID;

	public List<ShaderVariable> VariableList = new List<ShaderVariable>();

	public string SourceCode;

	public List<string> PostFunctions = new List<string>();

	public List<string> Flags = new List<string>();

	public ShaderPipeline Pipeline;

	private uint ShaderIndexValue;

	private bool ShaderIndexSet;

	private uint ShaderComboIndexValue;

	private bool ShaderComboIndexSet;

	private string SourceCodeDuplicateIDValue = "";

	private bool SourceCodeDuplicateIDSet;

	public uint ShaderIndex
	{
		get
		{
			if (ShaderIndexSet)
			{
				return ShaderIndexValue;
			}
			throw new Exception("Internal error: ShaderIndex accessed before set.");
		}
		set
		{
			ShaderIndexValue = value;
			ShaderIndexSet = true;
		}
	}

	public uint ShaderComboIndex
	{
		get
		{
			if (ShaderComboIndexSet)
			{
				return ShaderComboIndexValue;
			}
			throw new Exception("Internal error: ShaderComboIndex accessed before set.");
		}
		set
		{
			ShaderComboIndexValue = value;
			ShaderComboIndexSet = true;
		}
	}

	public string SourceCodeDuplicateID
	{
		get
		{
			if (SourceCodeDuplicateIDSet)
			{
				return SourceCodeDuplicateIDValue;
			}
			throw new Exception("Internal error: SourceCodeDuplicateID accessed before set.");
		}
		set
		{
			SourceCodeDuplicateIDValue = value;
			SourceCodeDuplicateIDSet = true;
		}
	}

	public uint UniformSize => (uint)VariableList.Sum((ShaderVariable var) => (var.VarType == ShaderVariable.VariableType.Variable_Uniform) ? var.RegisterCount : 0);

	public List<ShaderVariable> SortedAttributeList
	{
		get
		{
			List<ShaderVariable> list = VariableList.FindAll((ShaderVariable v) => v.VarType == ShaderVariable.VariableType.Variable_Attribute);
			string[] semanticOrder = new string[5] { "POSITION", "COLOR", "FACTOR", "TEXCOORD", "INSTANCE" };
			list.Sort(delegate(ShaderVariable s0, ShaderVariable s1)
			{
				int num = 0;
				int num2 = 0;
				for (int i = 0; i < semanticOrder.Length; i++)
				{
					if (s0.Semantic.StartsWith(semanticOrder[i]))
					{
						num = i;
					}
					if (s1.Semantic.StartsWith(semanticOrder[i]))
					{
						num2 = i;
					}
				}
				if (num == num2)
				{
					string value = Regex.Replace(s0.Semantic, "^[^0-9]+", "");
					string value2 = Regex.Replace(s1.Semantic, "^[^0-9]+", "");
					return Convert.ToInt32(value).CompareTo(Convert.ToInt32(value2));
				}
				return num.CompareTo(num2);
			});
			return list;
		}
	}

	public override int GetHashCode()
	{
		int num = SourceCode.GetHashCode() ^ Pipeline.GetHashCode();
		foreach (int item in Flags.ConvertAll((string f) => f.GetHashCode()))
		{
			num ^= item;
		}
		return num;
	}
}
