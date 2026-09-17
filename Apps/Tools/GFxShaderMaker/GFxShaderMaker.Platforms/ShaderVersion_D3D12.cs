using System;
using System.Collections.Generic;

namespace GFxShaderMaker.Platforms;

public class ShaderVersion_D3D12 : ShaderVersion_SM50
{
	public override string RootSignature => "#define ScaleformRS \"RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT),CBV(b0, visibility=SHADER_VISIBILITY_VERTEX),CBV(b1, visibility=SHADER_VISIBILITY_PIXEL),DescriptorTable(SRV(t0, numDescriptors=" + 4 + "), visibility=SHADER_VISIBILITY_PIXEL),DescriptorTable(Sampler(s0, numDescriptors=" + 4 + "), visibility=SHADER_VISIBILITY_PIXEL)\"\n\n";

	public override string RootSignatureAttribute => "[RootSignature(ScaleformRS)]\n";

	public ShaderVersion_D3D12(ShaderPlatform platform)
		: base(platform, "D3D12")
	{
	}

	protected override void writeSourceUniforms(ref string shaderCode, ShaderLinkedSource src, List<ShaderVariable> uniforms)
	{
		if (uniforms.Count <= 0)
		{
			return;
		}
		object obj = shaderCode;
		shaderCode = string.Concat(obj, "cbuffer Constants : register(b", Convert.ToInt32(src.Pipeline.Type), ")\n{\n");
		foreach (ShaderVariable uniform in uniforms)
		{
			object obj2 = shaderCode;
			shaderCode = string.Concat(obj2, uniform.Type, " ", uniform.ID, (uniform.ArraySize > 1) ? ("[" + uniform.ArraySize + "]") : "", " : packoffset(c", uniform.BaseRegister, ");\n");
		}
		shaderCode += "};\n\n";
	}
}
