using System.Collections.Generic;

namespace GFxShaderMaker.Platforms;

internal class ShaderVersion_X360 : ShaderVersion_SM30
{
	public ShaderVersion_X360(ShaderPlatform platform)
		: base(platform, "X360")
	{
	}

	public override void PostLink_Instanced(ShaderLinkedSource linkedSrc)
	{
		if (linkedSrc.Pipeline.Type != ShaderPipeline.PipelineType.Vertex)
		{
			return;
		}
		base.PostLink_Instanced(linkedSrc);
		string text = "\r\n    int4 indexPacked;\r\n    int unscaledIndexVertIdx = ((idx + 0.5f) % (int)(instSize.w + 0.5f));\r\n    int indexVertIdx = (unscaledIndexVertIdx)/2; // Account for 16 byte indices\r\n    int sampleComponent = unscaledIndexVertIdx % 2;\r\n    asm { vfetch indexPacked, indexVertIdx, color7 };\r\n    int vbatch = (idx + 0.5f) / instSize.w;\r\n    int meshidx = indexPacked[sampleComponent];\r\n\r\n";
		List<ShaderVariable> list = linkedSrc.VariableList.FindAll((ShaderVariable v) => v.VarType == ShaderVariable.VariableType.Variable_Attribute);
		list.ForEach(delegate(ShaderVariable v)
		{
			v.VarType = ShaderVariable.VariableType.Variable_VirtualAttribute;
		});
		Dictionary<string, uint> dictionary = new Dictionary<string, uint>();
		foreach (ShaderVariable item in list)
		{
			string text2 = item.Semantic;
			if (text2 == "FACTOR")
			{
				text2 = "COLOR";
			}
			if (!(text2 == "INSTANCE"))
			{
				uint value = 0u;
				if (dictionary.TryGetValue(text2, out value))
				{
					value++;
				}
				dictionary[text2] = value;
				text2 = (text2 + value).ToLower();
				string text3 = text;
				text = text3 + "    float4 " + item.ID + ";\n    asm { vfetch " + item.ID + ", meshidx, " + text2 + "};\n";
			}
		}
		ShaderVariable shaderVariable = new ShaderVariable();
		shaderVariable.ID = "idx";
		shaderVariable.VarType = ShaderVariable.VariableType.Variable_Attribute;
		shaderVariable.Type = "int";
		shaderVariable.Semantic = "INDEX";
		linkedSrc.VariableList.Add(shaderVariable);
		ShaderVariable shaderVariable2 = new ShaderVariable();
		shaderVariable2.ID = "instSize";
		shaderVariable2.VarType = ShaderVariable.VariableType.Variable_Uniform;
		shaderVariable2.Type = "float4";
		shaderVariable2.Semantic = "";
		linkedSrc.VariableList.Add(shaderVariable2);
		linkedSrc.SourceCode = text + linkedSrc.SourceCode;
	}
}
