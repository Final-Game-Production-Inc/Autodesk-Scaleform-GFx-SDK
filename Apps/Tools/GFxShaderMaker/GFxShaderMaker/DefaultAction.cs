using System;
using System.IO;
using System.Xml;

namespace GFxShaderMaker;

public class DefaultAction : CommandLineAction
{
	public void Execute()
	{
		string platformName = CommandLineParser.GetOption(CommandLineParser.Options.Platform);
		if (string.IsNullOrEmpty(platformName))
		{
			throw new Exception("Platform must be provided (use -platform).");
		}
		ShaderPlatform shaderPlatform = ShaderPlatform.PlatformList.Find((ShaderPlatform plt) => plt.PlatformName == platformName);
		if (shaderPlatform == null)
		{
			throw new Exception("Unrecognized platform: " + platformName + " (use -list to see possible platforms).");
		}
		string option = CommandLineParser.GetOption(CommandLineParser.Options.SourceXML);
		if (string.IsNullOrEmpty(option) || !File.Exists(option))
		{
			throw new Exception("Invalid source XML, or does not exist: " + option + ". (use -xml to specify).");
		}
		XmlDocument xmlDocument = new XmlDocument();
		xmlDocument.Load(option);
		shaderPlatform.ShaderBuildEvent(ShaderPlatform.ShaderBuildEventType.ShaderBuildEvent_Initialize);
		shaderPlatform.ReadFromXml(xmlDocument.DocumentElement);
		shaderPlatform.WriteShaderSources();
		if (!CommandLineParser.GetOption<bool>(CommandLineParser.Options.SkipHeaderAndSourceRegeneration))
		{
			shaderPlatform.WriteHeaderFile();
			shaderPlatform.WriteSourceFile();
		}
		shaderPlatform.ShaderBuildEvent(ShaderPlatform.ShaderBuildEventType.ShaderBuildEvent_PostShaderDesc);
		if (!CommandLineParser.GetOption<bool>(CommandLineParser.Options.SkipShaderRegeneration))
		{
			shaderPlatform.CreateShaderOutput();
		}
		shaderPlatform.ShaderBuildEvent(ShaderPlatform.ShaderBuildEventType.ShaderBuildEvent_Finalize);
	}
}
