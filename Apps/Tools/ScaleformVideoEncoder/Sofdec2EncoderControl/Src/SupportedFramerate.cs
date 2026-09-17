using System.Runtime.InteropServices;

namespace Sofdec2EncoderControl;

[StructLayout(LayoutKind.Sequential, Size = 1)]
public struct SupportedFramerate
{
	public const string frate60 = "60";

	public const string frate5994 = "59.94";

	public const string frate50 = "50";

	public const string frate30 = "30";

	public const string frate2997 = "29.97";

	public const string frate25 = "25";

	public const string frate24 = "24";

	public const string frate2397 = "23.98";

	public const string frate20 = "20";

	public const string frate15 = "15";

	public const string frate10 = "10";
}
