using System;
using System.IO;
using System.Linq;

namespace GFxShaderMaker;

public class IndentStreamWriter
{
	private StreamWriter Writer;

	public char[] IndentCharacters { get; set; }

	public char[] OutdentCharacters { get; set; }

	public int IndentSpaces { get; set; }

	public string CurrentIndent { get; set; }

	private bool NewLine { get; set; }

	public IndentStreamWriter(StreamWriter writer)
	{
		Writer = writer;
		IndentCharacters = "{".ToCharArray();
		OutdentCharacters = "}".ToCharArray();
		IndentSpaces = 4;
		CurrentIndent = "";
		NewLine = false;
	}

	public void Write(string str)
	{
		string text = str;
		int num = 0;
		if (NewLine)
		{
			if (text.Count() > 0 && OutdentCharacters.Contains(text[0]))
			{
				int num2 = Math.Min(IndentSpaces, CurrentIndent.Count());
				if (num2 > 0)
				{
					CurrentIndent = CurrentIndent.Remove(0, num2);
				}
				num += CurrentIndent.Count() + 1;
			}
			text = CurrentIndent + text;
		}
		NewLine = false;
		for (int i = num; i < text.Count(); i++)
		{
			if (text[i].Equals('\n'))
			{
				if (i == text.Count() - 1)
				{
					NewLine = true;
				}
				else if (OutdentCharacters.Contains(text[i + 1]))
				{
					int num3 = Math.Min(IndentSpaces, CurrentIndent.Count());
					if (num3 > 0)
					{
						CurrentIndent = CurrentIndent.Remove(0, num3);
					}
					i++;
				}
				else if (text[i + 1] != '\n')
				{
					text = text.Insert(i + 1, CurrentIndent);
				}
			}
			else if (IndentCharacters.Contains(text[i]))
			{
				CurrentIndent += "".PadLeft(IndentSpaces);
			}
			else if (OutdentCharacters.Contains(text[i]))
			{
				int num4 = Math.Min(IndentSpaces, CurrentIndent.Count());
				if (num4 > 0)
				{
					CurrentIndent = CurrentIndent.Remove(0, num4);
				}
			}
		}
		Writer.Write(text);
	}
}
