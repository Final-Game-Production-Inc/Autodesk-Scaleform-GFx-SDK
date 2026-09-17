namespace Sofdec2EncoderControl;

public struct AudioParameters
{
	private InputAudioType audioType;

	private string filepathMonoSte;

	private string filepathCenter;

	private string filepathLeft;

	private string filepathLeftSurround;

	private string filepathRight;

	private string filepathRightSurround;

	private string filepathLFE;

	public InputAudioType AudioType
	{
		get
		{
			return audioType;
		}
		set
		{
			audioType = value;
		}
	}

	public string FilePathMonoStereo
	{
		get
		{
			return filepathMonoSte;
		}
		set
		{
			filepathMonoSte = value;
		}
	}

	public string FilePath51Center
	{
		get
		{
			return filepathCenter;
		}
		set
		{
			filepathCenter = value;
		}
	}

	public string FilePath51LFE
	{
		get
		{
			return filepathLFE;
		}
		set
		{
			filepathLFE = value;
		}
	}

	public string FilePath51Left
	{
		get
		{
			return filepathLeft;
		}
		set
		{
			filepathLeft = value;
		}
	}

	public string FilePath51Right
	{
		get
		{
			return filepathRight;
		}
		set
		{
			filepathRight = value;
		}
	}

	public string FilePath51LeftSurround
	{
		get
		{
			return filepathLeftSurround;
		}
		set
		{
			filepathLeftSurround = value;
		}
	}

	public string FilePath51RightSurround
	{
		get
		{
			return filepathRightSurround;
		}
		set
		{
			filepathRightSurround = value;
		}
	}
}
