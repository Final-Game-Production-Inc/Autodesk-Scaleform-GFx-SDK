using System;
using System.Text;

namespace Sofdec2EncoderControl;

public class EncodingParameters
{
	private const string s_input_file = "input_file";

	private const string s_output_file = "output_file";

	private const string s_cuepoints_file = "cuepoints_file";

	private const string s_bitrate = "bitrate";

	private const string s_framerate_base = "framerate_base";

	private const string s_framerate_scale = "framerate_scale";

	private const string s_use_input_framerate = "use_input_framerate";

	private const string s_use_audio_track = "use_audio_track";

	private const string s_use_alpha_channel = "use_alpha_channel";

	private const string s_enable_resize = "enable_resize";

	private const string s_resize_width = "resize_width";

	private const string s_resize_height = "resize_height";

	private const string s_use_hca_codec = "use_use_hca_codec";

	private const string s_hca_quality = "hca_quality";

	private const string s_use_h264_codec = "use_h264_codec";

	private const string s_h264_target = "h264_target";

	private const string s_h264_profile = "h264_profile";

	private const string s_subtitles = "subtitles";

	private const string s_audio_type = "audio_type";

	private const string s_audio_file = "audio_file";

	private const string s_audio_file_C = "audio_file_C";

	private const string s_audio_file_L = "audio_file_L";

	private const string s_audio_file_LS = "audio_file_LS";

	private const string s_audio_file_R = "audio_file_R";

	private const string s_audio_file_RS = "audio_file_RS";

	private const string s_audio_file_LFE = "audio_file_LFE";

	public const int numberMaxAudioTracks = 32;

	public const int numberMaxSubtitleChannels = 32;

	public const string defaultFramerate = "29.97";

	public string inputVideoFilePath;

	public string outputFilePath;

	public string cuepointFilePath;

	public string[] subtitleFilePaths;

	public AudioParameters[] langParams;

	public int bitrate;

	public float framerateBase;

	public float framerateScale;

	public bool useAudioTrack;

	public bool useAlphaChannel;

	public bool useInputFramerate;

	public bool enableResize;

	public int resizeWidth;

	public int resizeHeight;

	public bool useHCA;

	public int hcaQuality;

	public bool useH264;

	public int h264Target;

	public int h264Profile;

	public bool isTargetPS2;

	public EncodingParameters()
	{
		langParams = new AudioParameters[32];
		subtitleFilePaths = new string[32];
		bitrate = 3600;
		framerateBase = 30000f;
		framerateScale = 1001f;
		inputVideoFilePath = null;
		outputFilePath = null;
		isTargetPS2 = false;
		useAudioTrack = false;
		useAlphaChannel = false;
		useInputFramerate = false;
		enableResize = false;
		resizeWidth = 0;
		resizeHeight = 0;
		useHCA = false;
		hcaQuality = 4;
		useH264 = false;
		h264Target = 1;
		h264Profile = 1;
	}

	public bool setFramerate(string frate)
	{
		switch (frate)
		{
		case "60":
			framerateBase = 60f;
			framerateScale = 1f;
			break;
		case "59.94":
			framerateBase = 60000f;
			framerateScale = 1001f;
			break;
		case "50":
			framerateBase = 50f;
			framerateScale = 1f;
			break;
		case "30":
			framerateBase = 30f;
			framerateScale = 1f;
			break;
		case "29.97":
			framerateBase = 30000f;
			framerateScale = 1001f;
			break;
		case "25":
			framerateBase = 25f;
			framerateScale = 1f;
			break;
		case "24":
			framerateBase = 24f;
			framerateScale = 1f;
			break;
		case "23.98":
			framerateBase = 24000f;
			framerateScale = 1001f;
			break;
		case "20":
			framerateBase = 20f;
			framerateScale = 1f;
			break;
		case "15":
			framerateBase = 15f;
			framerateScale = 1f;
			break;
		case "10":
			framerateBase = 10f;
			framerateScale = 1f;
			break;
		default:
			return false;
		}
		return true;
	}

	public void clearFramerate()
	{
		framerateBase = 0f;
		framerateScale = 0f;
	}

	public string getFramerate()
	{
		if (framerateScale == 0f)
		{
			return "29.97";
		}
		return $"{framerateBase / framerateScale:##.##}";
	}

	public bool export(out string buffer)
	{
		StringBuilder stringBuilder = new StringBuilder();
		addLine(stringBuilder, "input_file", inputVideoFilePath);
		addLine(stringBuilder, "output_file", outputFilePath);
		addLine(stringBuilder, "cuepoints_file", cuepointFilePath);
		for (int i = 0; i < 32; i++)
		{
			if (!string.IsNullOrEmpty(subtitleFilePaths[i]))
			{
				string text = i + "_";
				addLine(stringBuilder, text + "subtitles", subtitleFilePaths[i]);
			}
		}
		for (int j = 0; j < 32; j++)
		{
			AudioParameters audioParameters = langParams[j];
			if (audioParameters.AudioType != InputAudioType.None)
			{
				string text = j + "_";
				addLine(stringBuilder, text + "audio_type", (int)audioParameters.AudioType);
				switch (audioParameters.AudioType)
				{
				case InputAudioType.MonoOrStereo:
					addLine(stringBuilder, text + "audio_file", audioParameters.FilePathMonoStereo);
					break;
				case InputAudioType.MultiChannel:
					addLine(stringBuilder, text + "audio_file_C", audioParameters.FilePath51Center);
					addLine(stringBuilder, text + "audio_file_L", audioParameters.FilePath51Left);
					addLine(stringBuilder, text + "audio_file_LS", audioParameters.FilePath51LeftSurround);
					addLine(stringBuilder, text + "audio_file_R", audioParameters.FilePath51Right);
					addLine(stringBuilder, text + "audio_file_RS", audioParameters.FilePath51RightSurround);
					addLine(stringBuilder, text + "audio_file_LFE", audioParameters.FilePath51LFE);
					break;
				}
			}
		}
		addLine(stringBuilder, "bitrate", bitrate);
		addLine(stringBuilder, "framerate_base", framerateBase);
		addLine(stringBuilder, "framerate_scale", framerateScale);
		addLine(stringBuilder, "use_input_framerate", useInputFramerate);
		addLine(stringBuilder, "use_audio_track", useAudioTrack);
		addLine(stringBuilder, "use_alpha_channel", useAlphaChannel);
		addLine(stringBuilder, "enable_resize", enableResize);
		addLine(stringBuilder, "resize_width", resizeWidth);
		addLine(stringBuilder, "resize_height", resizeHeight);
		addLine(stringBuilder, "use_use_hca_codec", useHCA);
		addLine(stringBuilder, "hca_quality", hcaQuality);
		addLine(stringBuilder, "use_h264_codec", useH264);
		addLine(stringBuilder, "h264_target", h264Target);
		addLine(stringBuilder, "h264_profile", h264Profile);
		buffer = stringBuilder.ToString();
		return true;
	}

	public bool import(string[] lines)
	{
		if (lines == null)
		{
			return false;
		}
		foreach (string s in lines)
		{
			string key;
			string value;
			try
			{
				if (!parseLine(s, out key, out value))
				{
					continue;
				}
			}
			catch (Exception)
			{
				return false;
			}
			try
			{
				switch (key)
				{
				case "input_file":
					inputVideoFilePath = value;
					continue;
				case "output_file":
					outputFilePath = value;
					continue;
				case "cuepoints_file":
					cuepointFilePath = value;
					continue;
				case "bitrate":
					int.TryParse(value, out bitrate);
					continue;
				case "resize_width":
					int.TryParse(value, out resizeWidth);
					continue;
				case "resize_height":
					int.TryParse(value, out resizeHeight);
					continue;
				case "hca_quality":
					int.TryParse(value, out hcaQuality);
					continue;
				case "h264_target":
					int.TryParse(value, out h264Target);
					continue;
				case "h264_profile":
					int.TryParse(value, out h264Profile);
					continue;
				case "framerate_base":
					float.TryParse(value, out framerateBase);
					continue;
				case "framerate_scale":
					float.TryParse(value, out framerateScale);
					continue;
				case "use_input_framerate":
					useInputFramerate = toBool(value);
					continue;
				case "use_audio_track":
					useAudioTrack = toBool(value);
					continue;
				case "use_alpha_channel":
					useAlphaChannel = toBool(value);
					continue;
				case "enable_resize":
					enableResize = toBool(value);
					continue;
				case "use_use_hca_codec":
					useHCA = toBool(value);
					continue;
				case "use_h264_codec":
					useH264 = toBool(value);
					continue;
				}
				int num = key.IndexOf('_');
				if (num <= 0 || num >= key.Length - 1)
				{
					continue;
				}
				string s2 = key.Substring(0, num);
				if (!int.TryParse(s2, out var result))
				{
					continue;
				}
				string text = key.Substring(num + 1);
				switch (text)
				{
				case "subtitles":
					if (result >= 32)
					{
						return false;
					}
					subtitleFilePaths[result] = value;
					break;
				case "audio_type":
				case "audio_file":
				case "audio_file_C":
				case "audio_file_L":
				case "audio_file_LS":
				case "audio_file_R":
				case "audio_file_RS":
				case "audio_file_LFE":
					if (result >= 32)
					{
						return false;
					}
					switch (text)
					{
					case "audio_type":
						try
						{
							int audioType = int.Parse(value);
							langParams[result].AudioType = (InputAudioType)audioType;
						}
						catch (Exception)
						{
							return false;
						}
						break;
					case "audio_file":
						langParams[result].FilePathMonoStereo = value;
						break;
					case "audio_file_C":
						langParams[result].FilePath51Center = value;
						break;
					case "audio_file_L":
						langParams[result].FilePath51Left = value;
						break;
					case "audio_file_LS":
						langParams[result].FilePath51LeftSurround = value;
						break;
					case "audio_file_R":
						langParams[result].FilePath51Right = value;
						break;
					case "audio_file_RS":
						langParams[result].FilePath51RightSurround = value;
						break;
					case "audio_file_LFE":
						langParams[result].FilePath51LFE = value;
						break;
					}
					break;
				}
			}
			catch (Exception)
			{
				return false;
			}
		}
		return true;
	}

	private void addLine(StringBuilder sb, string key, string value)
	{
		sb.Append(key + " = " + value + Environment.NewLine);
	}

	private void addLine(StringBuilder sb, string key, int value)
	{
		addLine(sb, key, value.ToString());
	}

	private void addLine(StringBuilder sb, string key, double value)
	{
		addLine(sb, key, value.ToString());
	}

	private void addLine(StringBuilder sb, string key, bool value)
	{
		addLine(sb, key, value ? 1 : 0);
	}

	private bool parseLine(string s, out string key, out string value)
	{
		int num = s.IndexOf('=');
		key = s.Substring(0, num).Trim();
		value = s.Substring(num + 1).Trim();
		return key.Length > 0;
	}

	private bool toBool(string s)
	{
		int num = int.Parse(s);
		return num != 0;
	}
}
