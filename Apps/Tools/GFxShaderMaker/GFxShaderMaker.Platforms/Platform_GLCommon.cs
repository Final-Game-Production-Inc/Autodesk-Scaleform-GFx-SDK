namespace GFxShaderMaker.Platforms;

public abstract class Platform_GLCommon : ShaderPlatformSourceShaders
{
	protected override void writeHeaderShaderDescFunctions(IndentStreamWriter headerFile)
	{
		base.writeHeaderShaderDescFunctions(headerFile);
		headerFile.Write("static bool        UsesUniformBufferObjects(ShaderVersion ver);\n");
	}

	protected override void writeSourceShaderDescFunctions(IndentStreamWriter sourceFile)
	{
		base.writeSourceShaderDescFunctions(sourceFile);
		sourceFile.Write("bool ShaderDesc::UsesUniformBufferObjects(ShaderVersion ver)\n");
		sourceFile.Write("{\n");
		sourceFile.Write("switch(ver)\n");
		sourceFile.Write("{\n");
		foreach (ShaderVersion_GLSLCommon requestedShaderVersion in RequestedShaderVersions)
		{
			sourceFile.Write("case ShaderVersion_" + requestedShaderVersion.ID + ": return " + (requestedShaderVersion.UsesUniformBufferObjects ? "true" : "false") + ";\n");
		}
		sourceFile.Write("default: return false;\n");
		sourceFile.Write("}\n");
		sourceFile.Write("};\n\n");
	}
}
