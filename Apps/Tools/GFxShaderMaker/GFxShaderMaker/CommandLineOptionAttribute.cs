using System;

namespace GFxShaderMaker;

[AttributeUsage(AttributeTargets.Field)]
public class CommandLineOptionAttribute : Attribute
{
	public CmdLineOptionType Type;

	public string CommandFlag;

	public string Description;

	public Type ActionType;

	public string DefaultValue;

	public CommandLineOptionAttribute(CmdLineOptionType type, string flag, string defVal, string desc, Type actionType)
	{
		Type = type;
		CommandFlag = flag;
		Description = desc;
		ActionType = actionType;
		DefaultValue = defVal;
	}

	public override bool Match(object o)
	{
		string text = o.ToString();
		if (text[0] == '/' || text[0] == '-')
		{
			text = text.Remove(0, 1);
		}
		return string.Compare(text, CommandFlag, ignoreCase: true) == 0;
	}
}
