using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Windows.Forms;

namespace Sofdec2EncoderControl;

public class ParametersSettingPanel : UserControl
{
	public class EncodingRequestedEventArgs : EventArgs
	{
		private EncodingParameters encParam;

		public EncodingParameters EnvParam
		{
			get
			{
				return encParam;
			}
			set
			{
				encParam = value;
			}
		}

		public EncodingRequestedEventArgs(EncodingParameters encParam)
		{
			this.encParam = encParam;
		}
	}

	public class PreviewRequestedEventArgs : EventArgs
	{
		private string filename;

		private bool extendedPlayer;

		public string FileName
		{
			get
			{
				return filename;
			}
			set
			{
				filename = value;
			}
		}

		public bool ExtendedPlayer
		{
			get
			{
				return extendedPlayer;
			}
			set
			{
				extendedPlayer = value;
			}
		}

		public PreviewRequestedEventArgs(string filename, bool extendedPlayer)
		{
			FileName = filename;
			ExtendedPlayer = extendedPlayer;
		}
	}

	public class LoadRequestedEventArgs : EventArgs
	{
		private string filename;

		public string FileName
		{
			get
			{
				return filename;
			}
			set
			{
				filename = value;
			}
		}

		public LoadRequestedEventArgs(string filename)
		{
			FileName = filename;
		}
	}

	private const int MIN_RESIZE_WIDTH = 16;

	private const int MAX_RESIZE_WIDTH = 4088;

	private const int MIN_RESIZE_HEIGHT = 16;

	private const int MAX_RESIZE_HEIGHT = 4088;

	private const char SETTINGS_FILE_COMMENT = ';';

	private const string SETTINGS_FILE_HEADER = " SFVES 1.00 - Do not edit or delete this line";

	private const string EMPTY_FILE_SPEC = "";

	private const string INPUT_FILE_SPEC = " *.avi,*.bmp,*.tga,*.mp4";

	private const string OUTPUT_FILE_SPEC = "*.usm";

	private const string TEXT_FILE_SPEC = "*.txt";

	private const string AUDIO_FILE_SPEC = "*.wav";

	private int numberMaxTracks;

	private int numberMaxChannels;

	private AudioSettingSubPanel[] panelsInputAudio;

	private SubtitleSettingSubPanel[] panelsInputSubtitle;

	private bool isEncodingStarted;

	private bool isEncodingCanceled;

	private IContainer components;

	private Button btnEncode;

	private Button btnOutputFile;

	private TextBox tboxOutputFile;

	private GroupBox gbCompSettings;

	private TextBox tboxResizeHeight;

	private Label lblResizeX;

	private TextBox tboxResizeWidth;

	private CheckBox chboxResizeEnabled;

	private Label lblResize;

	private Label lblFps;

	private ComboBox comboxFramerate;

	private Label lblFramerate;

	private Label lblKbps;

	private Label lblBitrate;

	private Button btnCuePointFile;

	private TextBox tboxCuePointFile;

	private ComboBox comboxAudioTrackSelect;

	private Label lblAudioSelect;

	private GroupBox gbInputAudioMaterials;

	private CheckBox chboxUseAlphaCh;

	private CheckBox chboxUseAudioTrack;

	private Button btnInputVideoFile;

	private TextBox tboxInputVideoFile;

	private AudioSettingSubPanel dummyPanelInputAudio;

	private ComboBox comboxSubtitleChannelSelect;

	private GroupBox gbInputSubtitleFile;

	private Label label1;

	private SubtitleSettingSubPanel dummyPanelInputSubtitle;

	private Button btnPreview;

	private Label label4;

	private Label label3;

	private Label label2;

	private ProgressBar progbarEncode;

	private Label lblProgress;

	private RichTextBox richtboxStdout;

	private Button btnCancel;

	private Label lblEncodingLog;

	private Label lblProgressPercent;

	private RichTextBox richtboxStderr;

	private Label lblErrorLog;

	private GroupBox gbEncoding;

	private Label label5;

	private CheckBox chboxExtendedPreview;

	private CheckBox useInputFramerate;

	private Button btnLoadSettings;

	private Button btnSaveSettings;

	private ComboBox comboxBitrate;

	private CheckBox useHCA;

	private ComboBox comboxHCAQuality;

	private Label lblQuality;

	private GroupBox gbCodecSettings;

	private Label lblCodec;

    public RadioButton h264Windows;

    public RadioButton h264PS4;

	public RadioButton h264XboxOne;

	public RadioButton h264WiiU;

	private ComboBox comboxCodec;

	private Label label6;

	private ComboBox comboxH264Profile;

	public event EventHandler<EncodingRequestedEventArgs> EncodingRequestedEvent;

	public event EventHandler CancelRequestedEvent;

	public event EventHandler<PreviewRequestedEventArgs> PreviewRequestedEvent;

	public event EventHandler<LoadRequestedEventArgs> LoadRequestedEvent;

	public ParametersSettingPanel()
	{
		InitializeComponent();
		numberMaxTracks = 32;
		numberMaxChannels = 32;
		createMultiPanelsForAudioAndSubtitle();
	}

	private void EncodeParamControl_Load(object sender, EventArgs e)
	{
		comboxFramerate.SelectedIndex = 4;
		comboxBitrate.SelectedIndex = 5;
		comboxAudioTrackSelect.SelectedIndex = 0;
		comboxSubtitleChannelSelect.SelectedIndex = 0;
		comboxHCAQuality.SelectedIndex = 1;
		comboxCodec.SelectedIndex = 0;
		comboxH264Profile.SelectedIndex = 0;
	}

	private void createMultiPanelsForAudioAndSubtitle()
	{
		panelsInputAudio = new AudioSettingSubPanel[numberMaxTracks];
		SuspendLayout();
		for (int i = 0; i < numberMaxTracks; i++)
		{
			panelsInputAudio[i] = new AudioSettingSubPanel();
			panelsInputAudio[i].Font = dummyPanelInputAudio.Font;
			panelsInputAudio[i].Location = dummyPanelInputAudio.Location;
			panelsInputAudio[i].Margin = dummyPanelInputAudio.Margin;
			panelsInputAudio[i].Size = dummyPanelInputAudio.Size;
			panelsInputAudio[i].Name = "panelsInputLang" + i;
			panelsInputAudio[i].TabIndex = 50 + i;
			if (i == 0)
			{
				panelsInputAudio[i].Visible = true;
				panelsInputAudio[i].BringToFront();
			}
			else
			{
				panelsInputAudio[i].Visible = false;
			}
		}
		gbInputAudioMaterials.Controls.AddRange(panelsInputAudio);
		panelsInputSubtitle = new SubtitleSettingSubPanel[numberMaxChannels];
		for (int j = 0; j < numberMaxChannels; j++)
		{
			panelsInputSubtitle[j] = new SubtitleSettingSubPanel();
			panelsInputSubtitle[j].Location = dummyPanelInputSubtitle.Location;
			panelsInputSubtitle[j].Size = dummyPanelInputSubtitle.Size;
			panelsInputSubtitle[j].Name = "panelsInputSubtitle" + j;
			panelsInputSubtitle[j].TabIndex = 60 + j;
			if (j == 0)
			{
				panelsInputSubtitle[j].Visible = true;
				panelsInputSubtitle[j].BringToFront();
			}
			else
			{
				panelsInputSubtitle[j].Visible = false;
			}
		}
		gbInputSubtitleFile.Controls.AddRange(panelsInputSubtitle);
		ResumeLayout();
		gbInputAudioMaterials.Controls.Remove(dummyPanelInputAudio);
		gbInputSubtitleFile.Controls.Remove(dummyPanelInputSubtitle);
		dummyPanelInputAudio.Dispose();
		dummyPanelInputSubtitle.Dispose();
	}

	private bool enableEncodeButtonIfValid()
	{
		try
		{
			string directoryName = Path.GetDirectoryName(tboxOutputFile.Text.Trim().Trim('"'));
			btnEncode.Enabled = File.Exists(tboxInputVideoFile.Text.Trim().Trim('"')) && Directory.Exists(directoryName);
		}
		catch
		{
			btnEncode.Enabled = false;
		}
		return btnEncode.Enabled;
	}

	private bool doesFileExist(string filename)
	{
		return File.Exists(filename);
	}

	public bool warnIfFileNotExist(string filename)
	{
		if (doesFileExist(filename))
		{
			return true;
		}
		MessageBox.Show("This file does not exist.", "Warning", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
		return false;
	}

	private bool validateNumericValue(char ch)
	{
		if ((ch >= '0' && ch <= '9') || ch == '\b')
		{
			return true;
		}
		return false;
	}

	private void btnEncode_Click(object sender, EventArgs e)
	{
		EncodingParameters encodingParameters = createEncodeParameters();
		if (checkEncodeParameters(encodingParameters))
		{
			initEncodingLogPanel();
			btnEncode.Enabled = false;
			btnPreview.Enabled = false;
			isEncodingCanceled = false;
			Invoke(EncodingRequestedEvent, this, new EncodingRequestedEventArgs(encodingParameters));
		}
	}

	private void btnPreview_Click(object sender, EventArgs e)
	{
		if (PreviewRequestedEvent != null && File.Exists(tboxOutputFile.Text.Trim().Trim('"')))
		{
			PreviewRequestedEvent(this, new PreviewRequestedEventArgs(tboxOutputFile.Text.Trim().Trim('"'), chboxExtendedPreview.Checked));
		}
	}

	private bool checkEncodeParameters(EncodingParameters ep)
	{
		string text = Path.GetExtension(ep.inputVideoFilePath).ToLower();
		if (!text.Equals(".avi") && !text.Equals(".bmp") && !text.Equals(".tga") && !text.Equals(".mp4"))
		{
			MessageBox.Show("Only .avi, .bmp, .tga or .mp4 input files are supported at this time.", "Warning", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
			return false;
		}
		if (ep.enableResize && (ep.resizeWidth < 16 || ep.resizeWidth > 4088 || ep.resizeHeight < 16 || ep.resizeHeight > 4088))
		{
			MessageBox.Show("Invalid value specified for resized output.");
			return false;
		}
		return true;
	}

	private EncodingParameters createEncodeParameters()
	{
		EncodingParameters encodingParameters = new EncodingParameters();
		encodingParameters.inputVideoFilePath = tboxInputVideoFile.Text.Trim().Trim('"');
		encodingParameters.outputFilePath = tboxOutputFile.Text.Trim().Trim('"');
		encodingParameters.isTargetPS2 = false;
		encodingParameters.useAlphaChannel = chboxUseAlphaCh.Checked;
		encodingParameters.bitrate = Convert.ToInt32(comboxBitrate.Text);
		if (encodingParameters.useAlphaChannel)
		{
			encodingParameters.bitrate /= 2;
		}
		if (useInputFramerate.Checked)
		{
			encodingParameters.useInputFramerate = true;
			encodingParameters.clearFramerate();
		}
		else
		{
			encodingParameters.setFramerate(comboxFramerate.Text);
		}
		if (useHCA.Checked)
		{
			encodingParameters.useHCA = true;
			encodingParameters.hcaQuality = 5 - comboxHCAQuality.SelectedIndex;
		}
		if (comboxCodec.SelectedIndex == 1)
		{
			encodingParameters.useH264 = true;
            if (h264Windows.Checked)
            {
                encodingParameters.h264Target = 1;
            }
            if (h264XboxOne.Checked)
			{
				encodingParameters.h264Target = 2;
			}
			else if (h264PS4.Checked)
			{
				encodingParameters.h264Target = 3;
			}
			else if (h264WiiU.Checked)
			{
				encodingParameters.h264Target = 4;
			}
			encodingParameters.h264Profile = comboxH264Profile.SelectedIndex + 1;
		}
		if (chboxResizeEnabled.Checked)
		{
			int.TryParse(tboxResizeWidth.Text, out encodingParameters.resizeWidth);
			int.TryParse(tboxResizeHeight.Text, out encodingParameters.resizeHeight);
			encodingParameters.enableResize = true;
		}
		else
		{
			encodingParameters.enableResize = false;
		}
		for (int i = 0; i < numberMaxTracks; i++)
		{
			ref AudioParameters reference = ref encodingParameters.langParams[i];
			reference = panelsInputAudio[i].GetLanguageParamters();
		}
		for (int j = 0; j < numberMaxChannels; j++)
		{
			encodingParameters.subtitleFilePaths[j] = panelsInputSubtitle[j].GetSubtitleTextPath();
		}
		encodingParameters.cuepointFilePath = (File.Exists(tboxCuePointFile.Text.Trim().Trim('"')) ? tboxCuePointFile.Text.Trim().Trim('"') : string.Empty);
		return encodingParameters;
	}

	private void btnInputVideoFile_Click(object sender, EventArgs e)
	{
		OpenFileDialog openFileDialog = new OpenFileDialog();
		openFileDialog.Filter = "Video Source File(*.avi;*.bmp;*.tga;*.mp4)|*.avi;*.bmp;*.tga;*.mp4";
		openFileDialog.RestoreDirectory = true;
		if (openFileDialog.ShowDialog() == DialogResult.OK)
		{
			tboxInputVideoFile.Text = openFileDialog.FileName;
			Invoke(LoadRequestedEvent, this, new LoadRequestedEventArgs(tboxInputVideoFile.Text.Trim().Trim('"')));
		}
	}

	private void btnOutputFile_Click(object sender, EventArgs e)
	{
		SaveFileDialog saveFileDialog = new SaveFileDialog();
		saveFileDialog.Filter = "CRI Movie File(*.usm)|*.usm";
		saveFileDialog.RestoreDirectory = true;
		if (saveFileDialog.ShowDialog() == DialogResult.OK)
		{
			tboxOutputFile.Text = saveFileDialog.FileName;
		}
	}

	private void tboxOutputFile_TextChanged(object sender, EventArgs e)
	{
		enableEncodeButtonIfValid();
	}

	private void comboxInputAudio_SelectedIndexChanged(object sender, EventArgs e)
	{
		ComboBox comboBox = (ComboBox)sender;
		int selectedIndex = comboBox.SelectedIndex;
		for (int i = 0; i < numberMaxTracks; i++)
		{
			if (i == selectedIndex)
			{
				panelsInputAudio[i].Visible = true;
				panelsInputAudio[i].BringToFront();
			}
			else
			{
				panelsInputAudio[i].Visible = false;
			}
		}
	}

	private void tboxInputVideoFile_TextChanged(object sender, EventArgs e)
	{
		string path = tboxInputVideoFile.Text.Trim().Trim('"');
		string text = Path.GetExtension(path).ToLower();
		if (text.Equals(".avi") || (text.Equals(".mp4")))
		{
			chboxUseAlphaCh.Enabled = true;
			chboxUseAudioTrack.Enabled = true;
			chboxUseAudioTrack.Checked = true;
			panelsInputAudio[0].SetDefaultAudioTrack(tboxInputVideoFile.Text.Trim().Trim('"'));
			useInputFramerate.Enabled = true;
			useInputFramerate.Checked = true;
			comboxFramerate.Enabled = false;
		}
		else if (text.Equals(".bmp") || text.Equals(".tga"))
		{
			chboxUseAlphaCh.Enabled = true;
			chboxUseAudioTrack.Enabled = false;
			useInputFramerate.Checked = false;
			useInputFramerate.Enabled = false;
			comboxFramerate.Enabled = true;
		}
		else
		{
			chboxUseAlphaCh.Checked = false;
			chboxUseAudioTrack.Checked = false;
			chboxUseAlphaCh.Enabled = false;
			chboxUseAudioTrack.Enabled = false;
			useInputFramerate.Checked = false;
			useInputFramerate.Enabled = false;
			comboxFramerate.Enabled = true;
		}
		if (File.Exists(tboxInputVideoFile.Text.Trim().Trim('"')))
		{
			tboxOutputFile.Text = Path.ChangeExtension(path, ".usm");
			btnPreview.Enabled = File.Exists(tboxOutputFile.Text.Trim().Trim('"'));
		}
		else
		{
			btnPreview.Enabled = false;
		}
		enableEncodeButtonIfValid();
	}

	private void chboxUseAudioTrack_CheckedChanged(object sender, EventArgs e)
	{
		CheckBox checkBox = (CheckBox)sender;
		if (checkBox.Checked)
		{
			panelsInputAudio[0].SetDefaultAudioTrack(tboxInputVideoFile.Text.Trim().Trim('"'));
		}
		else
		{
			panelsInputAudio[0].ResetAudioTrack();
		}
	}

	private void comboxSubtitleChannelSelect_SelectedIndexChanged(object sender, EventArgs e)
	{
		ComboBox comboBox = (ComboBox)sender;
		int selectedIndex = comboBox.SelectedIndex;
		for (int i = 0; i < numberMaxTracks; i++)
		{
			if (i == selectedIndex)
			{
				panelsInputSubtitle[i].Visible = true;
				panelsInputSubtitle[i].BringToFront();
			}
			else
			{
				panelsInputSubtitle[i].Visible = false;
			}
		}
	}

	private void btnCuePointFile_Click(object sender, EventArgs e)
	{
		OpenFileDialog openFileDialog = new OpenFileDialog();
		openFileDialog.Filter = "Cue Point File(*.txt)|*.txt|All Files (*.*)|*.*";
		openFileDialog.RestoreDirectory = true;
		if (openFileDialog.ShowDialog() == DialogResult.OK)
		{
			tboxCuePointFile.Text = openFileDialog.FileName.Trim().Trim('"');
		}
	}

	private void tboxCuePointFile_TextChanged(object sender, EventArgs e)
	{
	}

	private void gbInputSubtitleFile_TextChanged(object sender, EventArgs e)
	{
	}

	private void initEncodingLogPanel()
	{
		isEncodingStarted = true;
		isEncodingCanceled = false;
		gbEncoding.Enabled = true;
		lblProgressPercent.Text = "0%";
		progbarEncode.Value = 0;
		btnCancel.Enabled = true;
		richtboxStdout.Clear();
		richtboxStderr.Clear();
	}

	public void AppendEncodingLogReceived(object sender, DataReceivedEventArgs e)
	{
		if (base.InvokeRequired)
		{
			Invoke(new DataReceivedEventHandler(AppendEncodingLogReceived), sender, e);
		}
		else if (isEncodingStarted)
		{
			appendEncodingLogText(e.Data + Environment.NewLine);
		}
	}

	public void AppendEncodingErrorReceived(object sender, DataReceivedEventArgs e)
	{
		if (base.InvokeRequired)
		{
			Invoke(new DataReceivedEventHandler(AppendEncodingErrorReceived), sender, e);
		}
		else
		{
			appendEncodingErrorText(richtboxStderr, e.Data + Environment.NewLine);
		}
	}

	public void EncodingProgressChanged(object sender, ProgressChangedEventArgs e)
	{
		if (base.InvokeRequired)
		{
			Invoke(new ProgressChangedEventHandler(EncodingProgressChanged), sender, e);
		}
		else
		{
			progbarEncode.Value = e.ProgressPercentage;
			lblProgressPercent.Text = $"{e.ProgressPercentage}%";
		}
	}

	public void EncodingExited(object sender, EventArgs e)
	{
		if (isEncodingStarted)
		{
			if (base.InvokeRequired)
			{
				Invoke(new EventHandler(EncodingExited), sender, e);
				return;
			}
			btnCancel.Enabled = false;
			btnEncode.Enabled = true;
			btnPreview.Enabled = File.Exists(tboxOutputFile.Text.Trim().Trim('"'));
			appendEncodingActionText("End Encoding!");
			isEncodingStarted = false;
		}
	}

	private void btnCancel_Click(object sender, EventArgs e)
	{
		if (!isEncodingCanceled && isEncodingStarted)
		{
			btnCancel.Enabled = false;
			isEncodingCanceled = true;
			if (isEncodingStarted && CancelRequestedEvent != null)
			{
				CancelRequestedEvent(this, null);
			}
			appendEncodingActionText("Encoding canceled.\n");
			progbarEncode.Value = 100;
			lblProgressPercent.Text = $"{100}%";
		}
	}

	private void appendLogText(RichTextBox richbox, string message)
	{
		richbox.AppendText(message);
		richbox.ScrollToCaret();
	}

	private void appendEncodingLogText(string message)
	{
		appendLogText(richtboxStdout, message);
	}

	private void appendEncodingErrorText(RichTextBox richbox, string message)
	{
		richbox.SelectionColor = Color.Red;
		Font font = (richbox.SelectionFont = new Font(richbox.SelectionFont.FontFamily, richbox.SelectionFont.Size + 1f, richbox.SelectionFont.Style | FontStyle.Bold));
		appendLogText(richbox, message);
		font.Dispose();
	}

	private void appendEncodingActionText(string message)
	{
		richtboxStdout.SelectionColor = Color.Blue;
		Font font = new Font(richtboxStdout.SelectionFont.FontFamily, richtboxStdout.SelectionFont.Size + 2f, richtboxStdout.SelectionFont.Style | FontStyle.Bold);
		richtboxStdout.SelectionFont = font;
		appendLogText(richtboxStdout, message);
		font.Dispose();
	}

	private void subPanelInputAudio_Load(object sender, EventArgs e)
	{
	}

	private void useInputFramerate_CheckedChanged(object sender, EventArgs e)
	{
		comboxFramerate.Enabled = !useInputFramerate.Checked;
	}

	private void dummyPanelInputSubtitle_Load(object sender, EventArgs e)
	{
	}

	private void tboxResizeWidth_KeyPress(object sender, KeyPressEventArgs e)
	{
		if (!validateNumericValue(e.KeyChar))
		{
			e.Handled = true;
		}
	}

	private void tboxResizeHeight_KeyPress(object sender, KeyPressEventArgs e)
	{
		if (!validateNumericValue(e.KeyChar))
		{
			e.Handled = true;
		}
	}

	private void btnSaveSettings_Click(object sender, EventArgs e)
	{
		loadSaveOptions(saving: true);
	}

	private void btnLoadSettings_Click(object sender, EventArgs e)
	{
		loadSaveOptions(saving: false);
	}

	private bool loadSaveOptions(bool saving)
	{
		FileDialog fileDialog = ((!saving) ? ((FileDialog)new OpenFileDialog()) : ((FileDialog)new SaveFileDialog()));
		fileDialog.Filter = "Encoder Settings File (*.sves)|*.sves";
		fileDialog.RestoreDirectory = true;
		if (fileDialog.ShowDialog() != DialogResult.OK)
		{
			return false;
		}
		if (saving)
		{
			return saveEncoderOptions(fileDialog.FileName);
		}
		return loadEncoderOptions(fileDialog.FileName);
	}

	private bool saveEncoderOptions(string filename)
	{
		EncodingParameters encodingParameters = new EncodingParameters();
		setParamsFromGUI(encodingParameters);
		encodingParameters.export(out var buffer);
		try
		{
			File.WriteAllText(filename, ';' + " SFVES 1.00 - Do not edit or delete this line" + Environment.NewLine);
			File.AppendAllText(filename, buffer);
		}
		catch (Exception)
		{
			MessageBox.Show("There was an error saving the settings.");
			return false;
		}
		return true;
	}

	private bool loadEncoderOptions(string filename)
	{
		List<string> list = new List<string>();
		try
		{
			StreamReader streamReader;
			using (streamReader = new StreamReader(filename))
			{
				string text;
				while ((text = streamReader.ReadLine()) != null)
				{
					int num = text.IndexOf(';');
					if (num != 0)
					{
						if (num > 0)
						{
							text = text.Substring(0, num);
						}
						text = text.Trim();
						if (text.Length > 0)
						{
							list.Add(text);
						}
					}
				}
			}
		}
		catch (Exception)
		{
			MessageBox.Show("There was an error loading the settings.");
			return false;
		}
		EncodingParameters encodingParameters = new EncodingParameters();
		if (!encodingParameters.import(list.ToArray()))
		{
			MessageBox.Show("Settings were in an invalid format.  Sorry.");
			return false;
		}
		setGUIFromParams(encodingParameters);
		return true;
	}

	private bool setParamsFromGUI(EncodingParameters ep)
	{
		ep.inputVideoFilePath = cleanFN(tboxInputVideoFile.Text.Trim().Trim('"'));
		ep.outputFilePath = cleanFN(tboxOutputFile.Text.Trim().Trim('"'));
		ep.cuepointFilePath = cleanFN(tboxCuePointFile.Text.Trim().Trim('"'));
		for (int i = 0; i < 32; i++)
		{
			ep.subtitleFilePaths[i] = cleanFN(panelsInputSubtitle[i].tboxSubtitleFile.Text);
		}
		for (int j = 0; j < 32; j++)
		{
			AudioSettingSubPanel audioSettingSubPanel = panelsInputAudio[j];
			AudioParameters audioParameters = default(AudioParameters);
			if (audioSettingSubPanel.rbtnNone.Checked)
			{
				audioParameters.AudioType = InputAudioType.None;
			}
			else if (audioSettingSubPanel.rbtnMonoStereo.Checked)
			{
				audioParameters.AudioType = InputAudioType.MonoOrStereo;
				audioParameters.FilePathMonoStereo = cleanFN(audioSettingSubPanel.tboxMonoStereo.Text);
			}
			else if (audioSettingSubPanel.rbtn51ch.Checked)
			{
				audioParameters.AudioType = InputAudioType.MultiChannel;
				audioParameters.FilePath51Center = cleanFN(audioSettingSubPanel.tBox51chC.Text);
				audioParameters.FilePath51Left = cleanFN(audioSettingSubPanel.tBox51chL.Text);
				audioParameters.FilePath51LeftSurround = cleanFN(audioSettingSubPanel.tBox51chLs.Text);
				audioParameters.FilePath51Right = cleanFN(audioSettingSubPanel.tBox51chR.Text);
				audioParameters.FilePath51RightSurround = cleanFN(audioSettingSubPanel.tBox51chRs.Text);
				audioParameters.FilePath51LFE = cleanFN(audioSettingSubPanel.tBox51chLFE.Text);
			}
			ep.langParams[j] = audioParameters;
		}
		ep.useAlphaChannel = chboxUseAlphaCh.Checked;
		ep.useAudioTrack = chboxUseAudioTrack.Checked;
		ep.bitrate = Convert.ToInt32(comboxBitrate.Text);
		ep.useInputFramerate = useInputFramerate.Checked;
		ep.setFramerate(comboxFramerate.SelectedItem.ToString());
		ep.useHCA = useHCA.Checked;
		ep.hcaQuality = 5 - comboxHCAQuality.SelectedIndex;
		ep.useH264 = comboxCodec.SelectedIndex == 1;
        if (h264Windows.Checked)
        {
            ep.h264Target = 1;
        }
        if (h264XboxOne.Checked)
		{
			ep.h264Target = 2;
		}
		else if (h264PS4.Checked)
		{
			ep.h264Target = 3;
		}
		else if (h264WiiU.Checked)
		{
			ep.h264Target = 4;
		}
		ep.h264Profile = 4 - comboxH264Profile.SelectedIndex;
		ep.enableResize = chboxResizeEnabled.Checked;
		int.TryParse(tboxResizeWidth.Text, out ep.resizeWidth);
		int.TryParse(tboxResizeHeight.Text, out ep.resizeHeight);
		return true;
	}

	private bool setGUIFromParams(EncodingParameters ep)
	{
		try
		{
			tboxInputVideoFile.Text = setFN(ep.inputVideoFilePath, " *.avi,*.bmp,*.tga,*mp4");
			tboxOutputFile.Text = setFN(ep.outputFilePath, "*.usm");
			tboxCuePointFile.Text = setFN(ep.cuepointFilePath, "*.txt");
			for (int i = 0; i < panelsInputSubtitle.Length; i++)
			{
				panelsInputSubtitle[i].tboxSubtitleFile.Text = setFN(ep.subtitleFilePaths[i], "*.txt");
			}
			for (int j = 0; j < panelsInputAudio.Length; j++)
			{
				_ = panelsInputAudio[j];
				AudioParameters audioParameters = ep.langParams[j];
				panelsInputAudio[j].tboxMonoStereo.Text = "*.wav";
				panelsInputAudio[j].tBox51chC.Text = "";
				panelsInputAudio[j].tBox51chL.Text = "";
				panelsInputAudio[j].tBox51chLs.Text = "";
				panelsInputAudio[j].tBox51chR.Text = "";
				panelsInputAudio[j].tBox51chRs.Text = "";
				panelsInputAudio[j].tBox51chLFE.Text = "";
				switch (audioParameters.AudioType)
				{
				default:
					panelsInputAudio[j].rbtnNone.Checked = true;
					break;
				case InputAudioType.MonoOrStereo:
					panelsInputAudio[j].rbtnMonoStereo.Checked = true;
					panelsInputAudio[j].tboxMonoStereo.Text = setFN(audioParameters.FilePathMonoStereo, "*.wav");
					break;
				case InputAudioType.MultiChannel:
					panelsInputAudio[j].rbtn51ch.Checked = true;
					panelsInputAudio[j].tBox51chC.Text = setFN(audioParameters.FilePath51Center, "");
					panelsInputAudio[j].tBox51chL.Text = setFN(audioParameters.FilePath51Left, "");
					panelsInputAudio[j].tBox51chLs.Text = setFN(audioParameters.FilePath51LeftSurround, "");
					panelsInputAudio[j].tBox51chR.Text = setFN(audioParameters.FilePath51Right, "");
					panelsInputAudio[j].tBox51chRs.Text = setFN(audioParameters.FilePath51RightSurround, "");
					panelsInputAudio[j].tBox51chLFE.Text = setFN(audioParameters.FilePath51LFE, "");
					break;
				}
			}
			chboxUseAlphaCh.Checked = ep.useAlphaChannel;
			chboxUseAudioTrack.Checked = ep.useAudioTrack;
			comboxBitrate.Text = Convert.ToString(ep.bitrate);
			useInputFramerate.Checked = ep.useInputFramerate;
			comboxFramerate.SelectedItem = ep.getFramerate();
			useHCA.Checked = ep.useHCA;
			comboxHCAQuality.SelectedIndex = 5 - ep.hcaQuality;
			if (ep.useH264)
			{
				comboxCodec.SelectedIndex = 1;
			}
			else
			{
				comboxCodec.SelectedIndex = 0;
			}
			switch (ep.h264Target)
			{
			case 1:
				h264XboxOne.Checked = true;
				break;
			case 2:
				h264PS4.Checked = true;
				break;
			case 3:
				h264WiiU.Checked = true;
				break;
			}
			comboxH264Profile.SelectedIndex = 3 - ep.h264Profile;
			chboxResizeEnabled.Checked = ep.enableResize;
			tboxResizeWidth.Text = ep.resizeWidth.ToString();
			tboxResizeHeight.Text = ep.resizeHeight.ToString();
		}
		catch (Exception)
		{
			return false;
		}
		return true;
	}

	private string cleanFN(string fn)
	{
		if (fn != null && !fn.Contains("*"))
		{
			return fn;
		}
		return "";
	}

	private string setFN(string fn, string def)
	{
		if (string.IsNullOrEmpty(fn))
		{
			return def;
		}
		return fn;
	}

	private void useHCA_CheckedChanged(object sender, EventArgs e)
	{
		comboxHCAQuality.Enabled = useHCA.Checked;
	}

	private void comboxCodec_SelectedIndexChanged(object sender, EventArgs e)
	{
		bool enabled = comboxCodec.SelectedIndex == 1;
		h264Windows.Enabled = enabled;
		h264XboxOne.Enabled = enabled;
		h264PS4.Enabled = enabled;
		h264WiiU.Enabled = false;
		comboxH264Profile.Enabled = enabled;
	}

	protected override void Dispose(bool disposing)
	{
		if (disposing && components != null)
		{
			components.Dispose();
		}
		base.Dispose(disposing);
	}

    private void InitializeComponent()
    {
        btnEncode = new Button();
        btnOutputFile = new Button();
        tboxOutputFile = new TextBox();
        gbCompSettings = new GroupBox();
        lblQuality = new Label();
        useHCA = new CheckBox();
        comboxHCAQuality = new ComboBox();
        comboxBitrate = new ComboBox();
        useInputFramerate = new CheckBox();
        label5 = new Label();
        tboxResizeHeight = new TextBox();
        lblResizeX = new Label();
        tboxResizeWidth = new TextBox();
        chboxResizeEnabled = new CheckBox();
        lblResize = new Label();
        lblFps = new Label();
        comboxFramerate = new ComboBox();
        lblFramerate = new Label();
        lblKbps = new Label();
        lblBitrate = new Label();
        btnCuePointFile = new Button();
        tboxCuePointFile = new TextBox();
        comboxAudioTrackSelect = new ComboBox();
        lblAudioSelect = new Label();
        gbInputAudioMaterials = new GroupBox();
        dummyPanelInputAudio = new AudioSettingSubPanel();
        label2 = new Label();
        comboxSubtitleChannelSelect = new ComboBox();
        gbInputSubtitleFile = new GroupBox();
        dummyPanelInputSubtitle = new SubtitleSettingSubPanel();
        label1 = new Label();
        label3 = new Label();
        tboxInputVideoFile = new TextBox();
        label4 = new Label();
        btnInputVideoFile = new Button();
        chboxUseAlphaCh = new CheckBox();
        chboxUseAudioTrack = new CheckBox();
        btnPreview = new Button();
        progbarEncode = new ProgressBar();
        lblProgress = new Label();
        richtboxStdout = new RichTextBox();
        btnCancel = new Button();
        lblEncodingLog = new Label();
        lblProgressPercent = new Label();
        richtboxStderr = new RichTextBox();
        lblErrorLog = new Label();
        gbEncoding = new GroupBox();
        chboxExtendedPreview = new CheckBox();
        btnLoadSettings = new Button();
        btnSaveSettings = new Button();
        gbCodecSettings = new GroupBox();
        label6 = new Label();
        comboxH264Profile = new ComboBox();
        comboxCodec = new ComboBox();
        h264Windows = new RadioButton();
        h264WiiU = new RadioButton();
        h264XboxOne = new RadioButton();
        h264PS4 = new RadioButton();
        lblCodec = new Label();
        gbCompSettings.SuspendLayout();
        gbInputAudioMaterials.SuspendLayout();
        gbInputSubtitleFile.SuspendLayout();
        gbEncoding.SuspendLayout();
        gbCodecSettings.SuspendLayout();
        SuspendLayout();
        // 
        // btnEncode
        // 
        btnEncode.Anchor = AnchorStyles.Bottom | AnchorStyles.Right;
        btnEncode.Enabled = false;
        btnEncode.Font = new Font("Arial", 8.25F, FontStyle.Bold, GraphicsUnit.Point, 0);
        btnEncode.Location = new Point(639, 685);
        btnEncode.Name = "btnEncode";
        btnEncode.Size = new Size(75, 26);
        btnEncode.TabIndex = 40;
        btnEncode.Text = "&Encode";
        btnEncode.UseVisualStyleBackColor = true;
        btnEncode.Click += btnEncode_Click;
        // 
        // btnOutputFile
        // 
        btnOutputFile.Font = new Font("Arial", 8.25F, FontStyle.Regular, GraphicsUnit.Point, 0);
        btnOutputFile.Location = new Point(352, 62);
        btnOutputFile.Name = "btnOutputFile";
        btnOutputFile.Size = new Size(24, 22);
        btnOutputFile.TabIndex = 5;
        btnOutputFile.Text = "...";
        btnOutputFile.UseVisualStyleBackColor = true;
        btnOutputFile.Click += btnOutputFile_Click;
        // 
        // tboxOutputFile
        // 
        tboxOutputFile.Font = new Font("Arial", 8.25F, FontStyle.Regular, GraphicsUnit.Point, 0);
        tboxOutputFile.Location = new Point(90, 63);
        tboxOutputFile.Name = "tboxOutputFile";
        tboxOutputFile.Size = new Size(257, 20);
        tboxOutputFile.TabIndex = 4;
        tboxOutputFile.Text = " *.usm";
        tboxOutputFile.TextChanged += tboxOutputFile_TextChanged;
        // 
        // gbCompSettings
        // 
        gbCompSettings.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
        gbCompSettings.Controls.Add(lblQuality);
        gbCompSettings.Controls.Add(useHCA);
        gbCompSettings.Controls.Add(comboxHCAQuality);
        gbCompSettings.Controls.Add(comboxBitrate);
        gbCompSettings.Controls.Add(useInputFramerate);
        gbCompSettings.Controls.Add(label5);
        gbCompSettings.Controls.Add(tboxResizeHeight);
        gbCompSettings.Controls.Add(lblResizeX);
        gbCompSettings.Controls.Add(tboxResizeWidth);
        gbCompSettings.Controls.Add(chboxResizeEnabled);
        gbCompSettings.Controls.Add(lblResize);
        gbCompSettings.Controls.Add(lblFps);
        gbCompSettings.Controls.Add(comboxFramerate);
        gbCompSettings.Controls.Add(lblFramerate);
        gbCompSettings.Controls.Add(lblKbps);
        gbCompSettings.Controls.Add(lblBitrate);
        gbCompSettings.Font = new Font("Arial", 8.25F, FontStyle.Bold, GraphicsUnit.Point, 0);
        gbCompSettings.ForeColor = SystemColors.WindowText;
        gbCompSettings.Location = new Point(398, 116);
        gbCompSettings.Name = "gbCompSettings";
        gbCompSettings.Size = new Size(399, 129);
        gbCompSettings.TabIndex = 27;
        gbCompSettings.TabStop = false;
        gbCompSettings.Text = "Video Settings";
        // 
        // lblQuality
        // 
        lblQuality.AutoSize = true;
        lblQuality.Font = new Font("Arial", 8.25F);
        lblQuality.Location = new Point(80, 101);
        lblQuality.Name = "lblQuality";
        lblQuality.Size = new Size(65, 14);
        lblQuality.TabIndex = 39;
        lblQuality.Text = "HCA quality:";
        // 
        // useHCA
        // 
        useHCA.AutoSize = true;
        useHCA.Font = new Font("Arial", 8.25F, FontStyle.Regular, GraphicsUnit.Point, 0);
        useHCA.Location = new Point(212, 76);
        useHCA.Name = "useHCA";
        useHCA.Size = new Size(163, 18);
        useHCA.TabIndex = 38;
        useHCA.Text = "Encode audio as HCA codec";
        useHCA.UseVisualStyleBackColor = true;
        useHCA.CheckedChanged += useHCA_CheckedChanged;
        // 
        // comboxHCAQuality
        // 
        comboxHCAQuality.DropDownStyle = ComboBoxStyle.DropDownList;
        comboxHCAQuality.Enabled = false;
        comboxHCAQuality.Font = new Font("Arial", 8.25F, FontStyle.Regular, GraphicsUnit.Point, 0);
        comboxHCAQuality.FormattingEnabled = true;
        comboxHCAQuality.Items.AddRange(new object[] { "Highest", "High (default)", "Medium", "Low (high compression)", "Lowest (highest compression)" });
        comboxHCAQuality.Location = new Point(154, 98);
        comboxHCAQuality.Name = "comboxHCAQuality";
        comboxHCAQuality.RightToLeft = RightToLeft.No;
        comboxHCAQuality.Size = new Size(197, 22);
        comboxHCAQuality.TabIndex = 37;
        // 
        // comboxBitrate
        // 
        comboxBitrate.AllowDrop = true;
        comboxBitrate.Font = new Font("Arial", 8.25F, FontStyle.Regular, GraphicsUnit.Point, 0);
        comboxBitrate.FormatString = "N0";
        comboxBitrate.FormattingEnabled = true;
        comboxBitrate.Items.AddRange(new object[] { "40000", "30000", "20000", "36000", "26000", "16000", "12000", "8000", "4000", "3600" });
        comboxBitrate.Location = new Point(83, 21);
        comboxBitrate.Name = "comboxBitrate";
        comboxBitrate.RightToLeft = RightToLeft.Yes;
        comboxBitrate.Size = new Size(65, 22);
        comboxBitrate.TabIndex = 36;
        // 
        // useInputFramerate
        // 
        useInputFramerate.AutoSize = true;
        useInputFramerate.Font = new Font("Arial", 8.25F, FontStyle.Regular, GraphicsUnit.Point, 0);
        useInputFramerate.Location = new Point(36, 76);
        useInputFramerate.Name = "useInputFramerate";
        useInputFramerate.Size = new Size(163, 18);
        useInputFramerate.TabIndex = 35;
        useInputFramerate.Text = "Use framerate from input file";
        useInputFramerate.UseVisualStyleBackColor = true;
        useInputFramerate.CheckedChanged += useInputFramerate_CheckedChanged;
        // 
        // label5
        // 
        label5.AutoSize = true;
        label5.Font = new Font("Arial", 8.25F);
        label5.Location = new Point(244, 51);
        label5.Name = "label5";
        label5.Size = new Size(14, 14);
        label5.TabIndex = 15;
        label5.Text = "H";
        // 
        // tboxResizeHeight
        // 
        tboxResizeHeight.Font = new Font("Arial", 8.25F, FontStyle.Regular, GraphicsUnit.Point, 0);
        tboxResizeHeight.Location = new Point(262, 48);
        tboxResizeHeight.Name = "tboxResizeHeight";
        tboxResizeHeight.Size = new Size(64, 20);
        tboxResizeHeight.TabIndex = 33;
        tboxResizeHeight.KeyPress += tboxResizeHeight_KeyPress;
        // 
        // lblResizeX
        // 
        lblResizeX.AutoSize = true;
        lblResizeX.Font = new Font("Arial", 8.25F);
        lblResizeX.Location = new Point(243, 25);
        lblResizeX.Name = "lblResizeX";
        lblResizeX.Size = new Size(17, 14);
        lblResizeX.TabIndex = 12;
        lblResizeX.Text = "W";
        // 
        // tboxResizeWidth
        // 
        tboxResizeWidth.Font = new Font("Arial", 8.25F, FontStyle.Regular, GraphicsUnit.Point, 0);
        tboxResizeWidth.Location = new Point(262, 22);
        tboxResizeWidth.Name = "tboxResizeWidth";
        tboxResizeWidth.Size = new Size(64, 20);
        tboxResizeWidth.TabIndex = 32;
        tboxResizeWidth.KeyPress += tboxResizeWidth_KeyPress;
        // 
        // chboxResizeEnabled
        // 
        chboxResizeEnabled.AutoSize = true;
        chboxResizeEnabled.Font = new Font("Arial", 8.25F, FontStyle.Regular, GraphicsUnit.Point, 0);
        chboxResizeEnabled.Location = new Point(332, 36);
        chboxResizeEnabled.Name = "chboxResizeEnabled";
        chboxResizeEnabled.Size = new Size(40, 18);
        chboxResizeEnabled.TabIndex = 34;
        chboxResizeEnabled.Text = "On";
        chboxResizeEnabled.UseVisualStyleBackColor = true;
        // 
        // lblResize
        // 
        lblResize.AutoSize = true;
        lblResize.Font = new Font("Arial", 8.25F);
        lblResize.Location = new Point(194, 23);
        lblResize.Name = "lblResize";
        lblResize.Size = new Size(43, 14);
        lblResize.TabIndex = 9;
        lblResize.Text = "Resize:";
        // 
        // lblFps
        // 
        lblFps.AutoSize = true;
        lblFps.Font = new Font("Arial", 8.25F, FontStyle.Regular, GraphicsUnit.Point, 0);
        lblFps.Location = new Point(151, 51);
        lblFps.Name = "lblFps";
        lblFps.Size = new Size(23, 14);
        lblFps.TabIndex = 8;
        lblFps.Text = "fps";
        // 
        // comboxFramerate
        // 
        comboxFramerate.DropDownStyle = ComboBoxStyle.DropDownList;
        comboxFramerate.Font = new Font("Arial", 8.25F, FontStyle.Regular, GraphicsUnit.Point, 0);
        comboxFramerate.FormattingEnabled = true;
        comboxFramerate.Items.AddRange(new object[] { "60", "59.94", "50", "30", "29.97", "25", "24", "23.98", "20", "15", "10" });
        comboxFramerate.Location = new Point(83, 48);
        comboxFramerate.Name = "comboxFramerate";
        comboxFramerate.RightToLeft = RightToLeft.Yes;
        comboxFramerate.Size = new Size(65, 22);
        comboxFramerate.TabIndex = 31;
        // 
        // lblFramerate
        // 
        lblFramerate.AutoSize = true;
        lblFramerate.Font = new Font("Arial", 8.25F);
        lblFramerate.Location = new Point(15, 51);
        lblFramerate.Name = "lblFramerate";
        lblFramerate.Size = new Size(59, 14);
        lblFramerate.TabIndex = 6;
        lblFramerate.Text = "Framerate:";
        // 
        // lblKbps
        // 
        lblKbps.AutoSize = true;
        lblKbps.Font = new Font("Arial", 8.25F, FontStyle.Regular, GraphicsUnit.Point, 0);
        lblKbps.Location = new Point(151, 24);
        lblKbps.Name = "lblKbps";
        lblKbps.Size = new Size(30, 14);
        lblKbps.TabIndex = 5;
        lblKbps.Text = "kbps";
        // 
        // lblBitrate
        // 
        lblBitrate.AutoSize = true;
        lblBitrate.Font = new Font("Arial", 8.25F);
        lblBitrate.Location = new Point(33, 25);
        lblBitrate.Name = "lblBitrate";
        lblBitrate.Size = new Size(41, 14);
        lblBitrate.TabIndex = 3;
        lblBitrate.Text = "Bitrate:";
        // 
        // btnCuePointFile
        // 
        btnCuePointFile.Font = new Font("Arial", 8.25F, FontStyle.Regular, GraphicsUnit.Point, 0);
        btnCuePointFile.Location = new Point(352, 88);
        btnCuePointFile.Name = "btnCuePointFile";
        btnCuePointFile.Size = new Size(24, 22);
        btnCuePointFile.TabIndex = 7;
        btnCuePointFile.Text = "...";
        btnCuePointFile.UseVisualStyleBackColor = true;
        btnCuePointFile.Click += btnCuePointFile_Click;
        // 
        // tboxCuePointFile
        // 
        tboxCuePointFile.Font = new Font("Arial", 8.25F, FontStyle.Regular, GraphicsUnit.Point, 0);
        tboxCuePointFile.Location = new Point(90, 89);
        tboxCuePointFile.Name = "tboxCuePointFile";
        tboxCuePointFile.Size = new Size(257, 20);
        tboxCuePointFile.TabIndex = 6;
        tboxCuePointFile.Text = " *.txt";
        tboxCuePointFile.TextChanged += tboxCuePointFile_TextChanged;
        // 
        // comboxAudioTrackSelect
        // 
        comboxAudioTrackSelect.DropDownStyle = ComboBoxStyle.DropDownList;
        comboxAudioTrackSelect.Font = new Font("Arial", 8.25F);
        comboxAudioTrackSelect.FormattingEnabled = true;
        comboxAudioTrackSelect.Items.AddRange(new object[] { "Track 0", "Track 1", "Track 2", "Track 3", "Track 4", "Track 5", "Track 6", "Track 7", "Track 8", "Track 9", "Track 10", "Track 11", "Track 12", "Track 13", "Track 14", "Track 15", "Track 16", "Track 17", "Track 18", "Track 19", "Track 20", "Track 21", "Track 22", "Track 23", "Track 24", "Track 25", "Track 26", "Track 27", "Track 28", "Track 29", "Track 30", "Track 31" });
        comboxAudioTrackSelect.Location = new Point(96, 187);
        comboxAudioTrackSelect.Name = "comboxAudioTrackSelect";
        comboxAudioTrackSelect.Size = new Size(115, 22);
        comboxAudioTrackSelect.TabIndex = 13;
        comboxAudioTrackSelect.SelectedIndexChanged += comboxInputAudio_SelectedIndexChanged;
        // 
        // lblAudioSelect
        // 
        lblAudioSelect.AutoSize = true;
        lblAudioSelect.Font = new Font("Arial", 8.25F, FontStyle.Bold, GraphicsUnit.Point, 0);
        lblAudioSelect.ForeColor = SystemColors.ControlText;
        lblAudioSelect.Location = new Point(10, 1);
        lblAudioSelect.Name = "lblAudioSelect";
        lblAudioSelect.Size = new Size(88, 14);
        lblAudioSelect.TabIndex = 3;
        lblAudioSelect.Text = "Other Audio:    ";
        // 
        // gbInputAudioMaterials
        // 
        gbInputAudioMaterials.Controls.Add(dummyPanelInputAudio);
        gbInputAudioMaterials.Controls.Add(lblAudioSelect);
        gbInputAudioMaterials.Font = new Font("Arial", 8.25F, FontStyle.Regular, GraphicsUnit.Point, 0);
        gbInputAudioMaterials.Location = new Point(7, 191);
        gbInputAudioMaterials.Name = "gbInputAudioMaterials";
        gbInputAudioMaterials.Size = new Size(379, 270);
        gbInputAudioMaterials.TabIndex = 23;
        gbInputAudioMaterials.TabStop = false;
        // 
        // dummyPanelInputAudio
        // 
        dummyPanelInputAudio.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
        dummyPanelInputAudio.Font = new Font("Arial", 9F, FontStyle.Regular, GraphicsUnit.Point, 0);
        dummyPanelInputAudio.Location = new Point(6, 26);
        dummyPanelInputAudio.Margin = new Padding(3, 4, 3, 4);
        dummyPanelInputAudio.Name = "dummyPanelInputAudio";
        dummyPanelInputAudio.Size = new Size(367, 243);
        dummyPanelInputAudio.TabIndex = 14;
        dummyPanelInputAudio.Load += subPanelInputAudio_Load;
        // 
        // label2
        // 
        label2.AutoSize = true;
        label2.Font = new Font("Arial", 8.25F);
        label2.Location = new Point(11, 92);
        label2.Name = "label2";
        label2.Size = new Size(74, 14);
        label2.TabIndex = 37;
        label2.Text = "Cue Point File:";
        // 
        // comboxSubtitleChannelSelect
        // 
        comboxSubtitleChannelSelect.DropDownStyle = ComboBoxStyle.DropDownList;
        comboxSubtitleChannelSelect.Font = new Font("Arial", 8.25F);
        comboxSubtitleChannelSelect.FormattingEnabled = true;
        comboxSubtitleChannelSelect.Items.AddRange(new object[] { "Track 0", "Track 1", "Track 2", "Track 3", "Track 4", "Track 5", "Track 6", "Track 7", "Track 8", "Track 9", "Track 10", "Track 11", "Track 12", "Track 13", "Track 14", "Track 15", "Track 16", "Track 17", "Track 18", "Track 19", "Track 20", "Track 21", "Track 22", "Track 23", "Track 24", "Track 25", "Track 26", "Track 27", "Track 28", "Track 29", "Track 30", "Track 31" });
        comboxSubtitleChannelSelect.Location = new Point(97, 119);
        comboxSubtitleChannelSelect.Name = "comboxSubtitleChannelSelect";
        comboxSubtitleChannelSelect.Size = new Size(115, 22);
        comboxSubtitleChannelSelect.TabIndex = 8;
        comboxSubtitleChannelSelect.SelectedIndexChanged += comboxSubtitleChannelSelect_SelectedIndexChanged;
        // 
        // gbInputSubtitleFile
        // 
        gbInputSubtitleFile.Controls.Add(dummyPanelInputSubtitle);
        gbInputSubtitleFile.Controls.Add(label1);
        gbInputSubtitleFile.Location = new Point(7, 125);
        gbInputSubtitleFile.Margin = new Padding(3, 4, 3, 4);
        gbInputSubtitleFile.Name = "gbInputSubtitleFile";
        gbInputSubtitleFile.Padding = new Padding(3, 4, 3, 4);
        gbInputSubtitleFile.Size = new Size(379, 55);
        gbInputSubtitleFile.TabIndex = 32;
        gbInputSubtitleFile.TabStop = false;
        gbInputSubtitleFile.TextChanged += gbInputSubtitleFile_TextChanged;
        // 
        // dummyPanelInputSubtitle
        // 
        dummyPanelInputSubtitle.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
        dummyPanelInputSubtitle.Location = new Point(12, 19);
        dummyPanelInputSubtitle.Margin = new Padding(1);
        dummyPanelInputSubtitle.Name = "dummyPanelInputSubtitle";
        dummyPanelInputSubtitle.Size = new Size(360, 31);
        dummyPanelInputSubtitle.TabIndex = 9;
        dummyPanelInputSubtitle.Load += dummyPanelInputSubtitle_Load;
        // 
        // label1
        // 
        label1.AutoSize = true;
        label1.Font = new Font("Arial", 8.25F, FontStyle.Bold, GraphicsUnit.Point, 0);
        label1.ForeColor = Color.Black;
        label1.Location = new Point(9, 0);
        label1.Name = "label1";
        label1.Size = new Size(93, 14);
        label1.TabIndex = 5;
        label1.Text = "Subtitle Text:     ";
        // 
        // label3
        // 
        label3.AutoSize = true;
        label3.Font = new Font("Arial", 8.25F, FontStyle.Bold, GraphicsUnit.Point, 0);
        label3.Location = new Point(13, 14);
        label3.Name = "label3";
        label3.Size = new Size(72, 14);
        label3.TabIndex = 36;
        label3.Text = "Input Name:";
        // 
        // tboxInputVideoFile
        // 
        tboxInputVideoFile.Font = new Font("Arial", 8.25F, FontStyle.Regular, GraphicsUnit.Point, 0);
        tboxInputVideoFile.Location = new Point(90, 11);
        tboxInputVideoFile.Name = "tboxInputVideoFile";
        tboxInputVideoFile.Size = new Size(257, 20);
        tboxInputVideoFile.TabIndex = 0;
        tboxInputVideoFile.Text = " *.avi,*.bmp,*.tga,*.mp4";
        tboxInputVideoFile.TextChanged += tboxInputVideoFile_TextChanged;
        // 
        // label4
        // 
        label4.AutoSize = true;
        label4.Font = new Font("Arial", 8.25F);
        label4.Location = new Point(13, 66);
        label4.Name = "label4";
        label4.Size = new Size(72, 14);
        label4.TabIndex = 16;
        label4.Text = "Output Name:";
        // 
        // btnInputVideoFile
        // 
        btnInputVideoFile.Font = new Font("Arial", 8.25F, FontStyle.Regular, GraphicsUnit.Point, 0);
        btnInputVideoFile.Location = new Point(352, 10);
        btnInputVideoFile.Name = "btnInputVideoFile";
        btnInputVideoFile.Size = new Size(24, 22);
        btnInputVideoFile.TabIndex = 1;
        btnInputVideoFile.Text = "...";
        btnInputVideoFile.UseVisualStyleBackColor = true;
        btnInputVideoFile.Click += btnInputVideoFile_Click;
        // 
        // chboxUseAlphaCh
        // 
        chboxUseAlphaCh.AutoSize = true;
        chboxUseAlphaCh.Enabled = false;
        chboxUseAlphaCh.Font = new Font("Arial", 8.25F, FontStyle.Regular, GraphicsUnit.Point, 0);
        chboxUseAlphaCh.Location = new Point(202, 37);
        chboxUseAlphaCh.Name = "chboxUseAlphaCh";
        chboxUseAlphaCh.Size = new Size(117, 18);
        chboxUseAlphaCh.TabIndex = 3;
        chboxUseAlphaCh.Text = "Use Alpha Channel";
        chboxUseAlphaCh.UseVisualStyleBackColor = true;
        // 
        // chboxUseAudioTrack
        // 
        chboxUseAudioTrack.AutoSize = true;
        chboxUseAudioTrack.Enabled = false;
        chboxUseAudioTrack.Font = new Font("Arial", 8.25F, FontStyle.Regular, GraphicsUnit.Point, 0);
        chboxUseAudioTrack.Location = new Point(90, 37);
        chboxUseAudioTrack.Name = "chboxUseAudioTrack";
        chboxUseAudioTrack.Size = new Size(105, 18);
        chboxUseAudioTrack.TabIndex = 2;
        chboxUseAudioTrack.Text = "Use Audio Track";
        chboxUseAudioTrack.UseVisualStyleBackColor = true;
        chboxUseAudioTrack.CheckedChanged += chboxUseAudioTrack_CheckedChanged;
        // 
        // btnPreview
        // 
        btnPreview.Anchor = AnchorStyles.Bottom | AnchorStyles.Right;
        btnPreview.Enabled = false;
        btnPreview.Font = new Font("Arial", 8.25F, FontStyle.Bold, GraphicsUnit.Point, 204);
        btnPreview.Location = new Point(720, 685);
        btnPreview.Name = "btnPreview";
        btnPreview.Size = new Size(75, 26);
        btnPreview.TabIndex = 41;
        btnPreview.Text = "&Preview";
        btnPreview.UseVisualStyleBackColor = true;
        btnPreview.Click += btnPreview_Click;
        // 
        // progbarEncode
        // 
        progbarEncode.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
        progbarEncode.Enabled = false;
        progbarEncode.Location = new Point(10, 40);
        progbarEncode.Name = "progbarEncode";
        progbarEncode.Size = new Size(313, 22);
        progbarEncode.TabIndex = 0;
        // 
        // lblProgress
        // 
        lblProgress.AutoSize = true;
        lblProgress.Font = new Font("Arial", 8.25F);
        lblProgress.Location = new Point(7, 23);
        lblProgress.Name = "lblProgress";
        lblProgress.Size = new Size(54, 14);
        lblProgress.TabIndex = 1;
        lblProgress.Text = "Progress:";
        // 
        // richtboxStdout
        // 
        richtboxStdout.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
        richtboxStdout.BorderStyle = BorderStyle.FixedSingle;
        richtboxStdout.Font = new Font("Courier New", 8.25F, FontStyle.Regular, GraphicsUnit.Point, 204);
        richtboxStdout.Location = new Point(10, 82);
        richtboxStdout.Name = "richtboxStdout";
        richtboxStdout.ReadOnly = true;
        richtboxStdout.Size = new Size(379, 215);
        richtboxStdout.TabIndex = 36;
        richtboxStdout.Text = "";
        richtboxStdout.WordWrap = false;
        // 
        // btnCancel
        // 
        btnCancel.Anchor = AnchorStyles.Top | AnchorStyles.Right;
        btnCancel.Font = new Font("Arial", 8.25F, FontStyle.Regular, GraphicsUnit.Point, 0);
        btnCancel.Location = new Point(329, 40);
        btnCancel.Name = "btnCancel";
        btnCancel.Size = new Size(60, 22);
        btnCancel.TabIndex = 35;
        btnCancel.Text = "&Cancel";
        btnCancel.UseVisualStyleBackColor = true;
        btnCancel.Click += btnCancel_Click;
        // 
        // lblEncodingLog
        // 
        lblEncodingLog.AutoSize = true;
        lblEncodingLog.Font = new Font("Arial", 8.25F);
        lblEncodingLog.Location = new Point(7, 65);
        lblEncodingLog.Name = "lblEncodingLog";
        lblEncodingLog.Size = new Size(75, 14);
        lblEncodingLog.TabIndex = 4;
        lblEncodingLog.Text = "Encoding Log:";
        // 
        // lblProgressPercent
        // 
        lblProgressPercent.BackColor = SystemColors.Control;
        lblProgressPercent.Font = new Font("Arial", 8.25F);
        lblProgressPercent.Location = new Point(66, 21);
        lblProgressPercent.Name = "lblProgressPercent";
        lblProgressPercent.Size = new Size(40, 17);
        lblProgressPercent.TabIndex = 5;
        lblProgressPercent.Text = "0%";
        lblProgressPercent.TextAlign = ContentAlignment.BottomRight;
        // 
        // richtboxStderr
        // 
        richtboxStderr.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
        richtboxStderr.BorderStyle = BorderStyle.FixedSingle;
        richtboxStderr.Font = new Font("Microsoft Sans Serif", 8.25F, FontStyle.Regular, GraphicsUnit.Point, 0);
        richtboxStderr.Location = new Point(10, 317);
        richtboxStderr.Name = "richtboxStderr";
        richtboxStderr.ReadOnly = true;
        richtboxStderr.Size = new Size(379, 80);
        richtboxStderr.TabIndex = 37;
        richtboxStderr.Text = "";
        richtboxStderr.WordWrap = false;
        // 
        // lblErrorLog
        // 
        lblErrorLog.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
        lblErrorLog.AutoSize = true;
        lblErrorLog.Font = new Font("Arial", 8.25F);
        lblErrorLog.Location = new Point(7, 300);
        lblErrorLog.Name = "lblErrorLog";
        lblErrorLog.Size = new Size(55, 14);
        lblErrorLog.TabIndex = 7;
        lblErrorLog.Text = "Error Log:";
        // 
        // gbEncoding
        // 
        gbEncoding.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
        gbEncoding.BackColor = SystemColors.Control;
        gbEncoding.Controls.Add(lblErrorLog);
        gbEncoding.Controls.Add(richtboxStderr);
        gbEncoding.Controls.Add(lblProgressPercent);
        gbEncoding.Controls.Add(lblEncodingLog);
        gbEncoding.Controls.Add(btnCancel);
        gbEncoding.Controls.Add(richtboxStdout);
        gbEncoding.Controls.Add(lblProgress);
        gbEncoding.Controls.Add(progbarEncode);
        gbEncoding.Enabled = false;
        gbEncoding.Font = new Font("Arial", 8.25F, FontStyle.Bold, GraphicsUnit.Point, 0);
        gbEncoding.ForeColor = SystemColors.WindowText;
        gbEncoding.Location = new Point(396, 251);
        gbEncoding.Name = "gbEncoding";
        gbEncoding.Size = new Size(399, 414);
        gbEncoding.TabIndex = 30;
        gbEncoding.TabStop = false;
        gbEncoding.Text = "Encoding";
        // 
        // chboxExtendedPreview
        // 
        chboxExtendedPreview.Anchor = AnchorStyles.Bottom | AnchorStyles.Right;
        chboxExtendedPreview.AutoSize = true;
        chboxExtendedPreview.Checked = true;
        chboxExtendedPreview.CheckState = CheckState.Checked;
        chboxExtendedPreview.Font = new Font("Arial", 8.25F, FontStyle.Regular, GraphicsUnit.Point, 0);
        chboxExtendedPreview.Location = new Point(441, 690);
        chboxExtendedPreview.Name = "chboxExtendedPreview";
        chboxExtendedPreview.Size = new Size(189, 18);
        chboxExtendedPreview.TabIndex = 42;
        chboxExtendedPreview.Text = "Preview in extended Video player";
        chboxExtendedPreview.UseVisualStyleBackColor = true;
        // 
        // btnLoadSettings
        // 
        btnLoadSettings.Anchor = AnchorStyles.Bottom | AnchorStyles.Left;
        btnLoadSettings.Font = new Font("Arial", 8.25F, FontStyle.Regular, GraphicsUnit.Point, 0);
        btnLoadSettings.Location = new Point(112, 685);
        btnLoadSettings.Name = "btnLoadSettings";
        btnLoadSettings.Size = new Size(115, 22);
        btnLoadSettings.TabIndex = 45;
        btnLoadSettings.Text = "&Load settings...";
        btnLoadSettings.UseVisualStyleBackColor = true;
        btnLoadSettings.Click += btnLoadSettings_Click;
        // 
        // btnSaveSettings
        // 
        btnSaveSettings.Anchor = AnchorStyles.Bottom | AnchorStyles.Left;
        btnSaveSettings.Location = new Point(13, 684);
        btnSaveSettings.Name = "btnSaveSettings";
        btnSaveSettings.Size = new Size(92, 23);
        btnSaveSettings.TabIndex = 46;
        btnSaveSettings.Text = "&Save settings...";
        btnSaveSettings.UseVisualStyleBackColor = true;
        btnSaveSettings.Click += btnSaveSettings_Click;
        // 
        // gbCodecSettings
        // 
        gbCodecSettings.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
        gbCodecSettings.Controls.Add(label6);
        gbCodecSettings.Controls.Add(comboxH264Profile);
        gbCodecSettings.Controls.Add(comboxCodec);
        gbCodecSettings.Controls.Add(h264Windows);
        gbCodecSettings.Controls.Add(h264WiiU);
        gbCodecSettings.Controls.Add(h264XboxOne);
        gbCodecSettings.Controls.Add(h264PS4);
        gbCodecSettings.Controls.Add(lblCodec);
        gbCodecSettings.Font = new Font("Arial", 8.25F, FontStyle.Bold, GraphicsUnit.Point, 0);
        gbCodecSettings.ForeColor = SystemColors.WindowText;
        gbCodecSettings.Location = new Point(396, 0);
        gbCodecSettings.Name = "gbCodecSettings";
        gbCodecSettings.Size = new Size(399, 110);
        gbCodecSettings.TabIndex = 47;
        gbCodecSettings.TabStop = false;
        gbCodecSettings.Text = "Codec";
        // 
        // label6
        // 
        label6.AutoSize = true;
        label6.Font = new Font("Arial", 8.25F);
        label6.Location = new Point(36, 77);
        label6.Name = "label6";
        label6.Size = new Size(40, 14);
        label6.TabIndex = 43;
        label6.Text = "Profile:";
        // 
        // comboxH264Profile
        // 
        comboxH264Profile.DropDownStyle = ComboBoxStyle.DropDownList;
        comboxH264Profile.Font = new Font("Arial", 8.25F, FontStyle.Regular, GraphicsUnit.Point, 0);
        comboxH264Profile.FormattingEnabled = true;
        comboxH264Profile.Items.AddRange(new object[] { "High (default)", "Main", "Baseline" });
        comboxH264Profile.Location = new Point(85, 74);
        comboxH264Profile.Name = "comboxH264Profile";
        comboxH264Profile.RightToLeft = RightToLeft.No;
        comboxH264Profile.Size = new Size(159, 22);
        comboxH264Profile.TabIndex = 42;
        // 
        // comboxCodec
        // 
        comboxCodec.DropDownStyle = ComboBoxStyle.DropDownList;
        comboxCodec.Font = new Font("Arial", 8.25F, FontStyle.Regular, GraphicsUnit.Point, 0);
        comboxCodec.FormattingEnabled = true;
        comboxCodec.Items.AddRange(new object[] { "Sofdec.Prime (default)", "H.264 (selected targets)" });
        comboxCodec.Location = new Point(85, 19);
        comboxCodec.Name = "comboxCodec";
        comboxCodec.RightToLeft = RightToLeft.No;
        comboxCodec.Size = new Size(159, 22);
        comboxCodec.TabIndex = 41;
        comboxCodec.SelectedIndexChanged += comboxCodec_SelectedIndexChanged;
        // 
        // h264Windows
        // 
        h264Windows.Checked = true;
        h264Windows.Font = new Font("Arial", 8.25F, FontStyle.Regular, GraphicsUnit.Point, 0);
        h264Windows.Location = new Point(38, 46);
        h264Windows.Name = "h264Windows";
        h264Windows.Size = new Size(72, 24);
        h264Windows.TabIndex = 44;
        h264Windows.TabStop = true;
        h264Windows.Text = "Windows";
        // 
        // h264WiiU
        // 
        h264WiiU.AutoSize = true;
        h264WiiU.Font = new Font("Arial", 8.25F, FontStyle.Regular, GraphicsUnit.Point, 0);
        h264WiiU.Location = new Point(300, 49);
        h264WiiU.Margin = new Padding(3, 4, 3, 4);
        h264WiiU.Name = "h264WiiU";
        h264WiiU.Size = new Size(49, 18);
        h264WiiU.TabIndex = 39;
        h264WiiU.Text = "Wii U";
        h264WiiU.UseVisualStyleBackColor = true;
        // 
        // h264XboxOne
        // 
        h264XboxOne.AutoSize = true;
        h264XboxOne.Font = new Font("Arial", 8.25F, FontStyle.Regular, GraphicsUnit.Point, 0);
        h264XboxOne.Location = new Point(122, 49);
        h264XboxOne.Margin = new Padding(3, 4, 3, 4);
        h264XboxOne.Name = "h264XboxOne";
        h264XboxOne.Size = new Size(73, 18);
        h264XboxOne.TabIndex = 38;
        h264XboxOne.Text = "Xbox One";
        h264XboxOne.UseVisualStyleBackColor = true;
        // 
        // h264PS4
        // 
        h264PS4.AutoSize = true;
        h264PS4.Font = new Font("Arial", 8.25F, FontStyle.Regular, GraphicsUnit.Point, 0);
        h264PS4.Location = new Point(204, 49);
        h264PS4.Margin = new Padding(3, 4, 3, 4);
        h264PS4.Name = "h264PS4";
        h264PS4.Size = new Size(87, 18);
        h264PS4.TabIndex = 37;
        h264PS4.Text = "PlayStation 4";
        h264PS4.UseVisualStyleBackColor = true;
        // 
        // lblCodec
        // 
        lblCodec.AutoSize = true;
        lblCodec.Font = new Font("Arial", 8.25F);
        lblCodec.Location = new Point(35, 22);
        lblCodec.Name = "lblCodec";
        lblCodec.Size = new Size(41, 14);
        lblCodec.TabIndex = 3;
        lblCodec.Text = "Codec:";
        // 
        // ParametersSettingPanel
        // 
        Controls.Add(gbCodecSettings);
        Controls.Add(btnSaveSettings);
        Controls.Add(btnLoadSettings);
        Controls.Add(chboxExtendedPreview);
        Controls.Add(comboxAudioTrackSelect);
        Controls.Add(btnPreview);
        Controls.Add(label2);
        Controls.Add(gbCompSettings);
        Controls.Add(btnEncode);
        Controls.Add(chboxUseAudioTrack);
        Controls.Add(gbEncoding);
        Controls.Add(tboxOutputFile);
        Controls.Add(comboxSubtitleChannelSelect);
        Controls.Add(chboxUseAlphaCh);
        Controls.Add(btnCuePointFile);
        Controls.Add(btnOutputFile);
        Controls.Add(btnInputVideoFile);
        Controls.Add(gbInputSubtitleFile);
        Controls.Add(gbInputAudioMaterials);
        Controls.Add(label4);
        Controls.Add(label3);
        Controls.Add(tboxInputVideoFile);
        Controls.Add(tboxCuePointFile);
        Name = "ParametersSettingPanel";
        Size = new Size(800, 720);
        Load += EncodeParamControl_Load;
        gbCompSettings.ResumeLayout(false);
        gbCompSettings.PerformLayout();
        gbInputAudioMaterials.ResumeLayout(false);
        gbInputAudioMaterials.PerformLayout();
        gbInputSubtitleFile.ResumeLayout(false);
        gbInputSubtitleFile.PerformLayout();
        gbEncoding.ResumeLayout(false);
        gbEncoding.PerformLayout();
        gbCodecSettings.ResumeLayout(false);
        gbCodecSettings.PerformLayout();
        ResumeLayout(false);
        PerformLayout();
    }
}
