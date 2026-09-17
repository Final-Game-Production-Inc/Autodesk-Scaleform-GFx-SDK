using System;
using System.ComponentModel;
using System.Drawing;
using System.IO;
using System.Windows.Forms;

namespace Sofdec2EncoderControl;

public class AudioSettingSubPanel : UserControl
{
	private IContainer components;

	public Panel panelMonoStereo;

	public Label lblMonoStereo;

	public Button btnMonoStereo;

	public TextBox tboxMonoStereo;

	public Panel panel51ch;

	public Label lbl51ch;

	public Label lbl51chRs;

	public TextBox tBox51chLs;

	public Label lbl51chR;

	public TextBox tBox51chC;

	public Label lbl51chLFE;

	public TextBox tBox51chL;

	public Label lbl51chLs;

	public TextBox tBox51chLFE;

	public Label lbl51chL;

	public TextBox tBox51chR;

	public Label lbl51chC;

	public TextBox tBox51chRs;

	public Button btn51chRs;

	public Button btn51chC;

	public Button btn51chR;

	public Button btn51chL;

	public Button btn51chLFE;

	public Button btn51chLs;

	public RadioButton rbtn51ch;

	public RadioButton rbtnMonoStereo;

	public RadioButton rbtnNone;

	public AudioSettingSubPanel()
	{
		InitializeComponent();
	}

	public void SetDefaultAudioTrack(string audioFilePath)
	{
		rbtnMonoStereo.Checked = true;
		tboxMonoStereo.Text = audioFilePath;
	}

	public void ResetAudioTrack()
	{
		rbtnNone.Checked = true;
		tboxMonoStereo.Text = string.Empty;
	}

	public AudioParameters GetLanguageParamters()
	{
		AudioParameters result = default(AudioParameters);
		if (rbtnMonoStereo.Checked)
		{
			if (checkFilePath(tboxMonoStereo.Text))
			{
				result.AudioType = InputAudioType.MonoOrStereo;
				result.FilePathMonoStereo = tboxMonoStereo.Text;
			}
			else
			{
				result.AudioType = InputAudioType.None;
			}
		}
		else if (rbtn51ch.Checked)
		{
			if (surroundDataPathsExist())
			{
				result.AudioType = InputAudioType.MultiChannel;
				result.FilePath51Center = tBox51chC.Text;
				result.FilePath51Left = tBox51chL.Text;
				result.FilePath51Right = tBox51chR.Text;
				result.FilePath51LeftSurround = tBox51chLs.Text;
				result.FilePath51RightSurround = tBox51chRs.Text;
				result.FilePath51LFE = tBox51chLFE.Text;
			}
			else
			{
				result.AudioType = InputAudioType.None;
			}
		}
		else
		{
			result.AudioType = InputAudioType.None;
		}
		return result;
	}

	private void rbtnNone_CheckedChanged(object sender, EventArgs e)
	{
		panelMonoStereo.Enabled = false;
		panel51ch.Enabled = false;
	}

	private void rbtnMonoStereo_CheckedChanged(object sender, EventArgs e)
	{
		panelMonoStereo.Enabled = true;
		panel51ch.Enabled = false;
	}

	private void rbtn51ch_CheckedChanged(object sender, EventArgs e)
	{
		panelMonoStereo.Enabled = false;
		panel51ch.Enabled = true;
	}

	private void btnMonoStereo_Click(object sender, EventArgs e)
	{
		OpenFileDialog openFileDialog = new OpenFileDialog();
		openFileDialog.Filter = "Wave Files(*.wav)|*.wav|All Files|*.*";
		openFileDialog.RestoreDirectory = true;
		if (openFileDialog.ShowDialog() == DialogResult.OK)
		{
			tboxMonoStereo.Text = openFileDialog.FileName;
		}
	}

	private void btn51chL_Click(object sender, EventArgs e)
	{
		OpenFileDialog openFileDialog = new OpenFileDialog();
		openFileDialog.Filter = "Mono Wave Files(*.wav)|*.wav";
		openFileDialog.RestoreDirectory = true;
		if (openFileDialog.ShowDialog() == DialogResult.OK)
		{
			tBox51chL.Text = openFileDialog.FileName;
		}
	}

	private void btn51chR_Click(object sender, EventArgs e)
	{
		OpenFileDialog openFileDialog = new OpenFileDialog();
		openFileDialog.Filter = "Mono Wave Files(*.wav)|*.wav";
		openFileDialog.RestoreDirectory = true;
		if (openFileDialog.ShowDialog() == DialogResult.OK)
		{
			tBox51chR.Text = openFileDialog.FileName;
		}
	}

	private void btn51chC_Click(object sender, EventArgs e)
	{
		OpenFileDialog openFileDialog = new OpenFileDialog();
		openFileDialog.Filter = "Mono Wave Files(*.wav)|*.wav";
		openFileDialog.RestoreDirectory = true;
		if (openFileDialog.ShowDialog() == DialogResult.OK)
		{
			tBox51chC.Text = openFileDialog.FileName;
		}
	}

	private void btn51chLFE_Click(object sender, EventArgs e)
	{
		OpenFileDialog openFileDialog = new OpenFileDialog();
		openFileDialog.Filter = "Mono Wave Files(*.wav)|*.wav";
		openFileDialog.RestoreDirectory = true;
		if (openFileDialog.ShowDialog() == DialogResult.OK)
		{
			tBox51chLFE.Text = openFileDialog.FileName;
		}
	}

	private void btn51chLs_Click(object sender, EventArgs e)
	{
		OpenFileDialog openFileDialog = new OpenFileDialog();
		openFileDialog.Filter = "Mono Wave Files(*.wav)|*.wav";
		openFileDialog.RestoreDirectory = true;
		if (openFileDialog.ShowDialog() == DialogResult.OK)
		{
			tBox51chLs.Text = openFileDialog.FileName;
		}
	}

	private void btn51chRs_Click(object sender, EventArgs e)
	{
		OpenFileDialog openFileDialog = new OpenFileDialog();
		openFileDialog.Filter = "Mono Wave Files(*.wav)|*.wav";
		openFileDialog.RestoreDirectory = true;
		if (openFileDialog.ShowDialog() == DialogResult.OK)
		{
			tBox51chRs.Text = openFileDialog.FileName;
		}
	}

	private static bool checkFilePath(string path)
	{
		char[] anyOf = new char[2] { '*', '?' };
		if (path.IndexOfAny(anyOf) >= 0)
		{
			return false;
		}
		if (!File.Exists(path))
		{
			return false;
		}
		return true;
	}

	private bool surroundDataPathsExist()
	{
		if (!checkFilePath(tBox51chC.Text))
		{
			return false;
		}
		if (!checkFilePath(tBox51chL.Text))
		{
			return false;
		}
		if (!checkFilePath(tBox51chR.Text))
		{
			return false;
		}
		if (!checkFilePath(tBox51chLs.Text))
		{
			return false;
		}
		if (!checkFilePath(tBox51chRs.Text))
		{
			return false;
		}
		return true;
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
		this.panelMonoStereo = new System.Windows.Forms.Panel();
		this.lblMonoStereo = new System.Windows.Forms.Label();
		this.btnMonoStereo = new System.Windows.Forms.Button();
		this.tboxMonoStereo = new System.Windows.Forms.TextBox();
		this.panel51ch = new System.Windows.Forms.Panel();
		this.lbl51ch = new System.Windows.Forms.Label();
		this.lbl51chRs = new System.Windows.Forms.Label();
		this.tBox51chLs = new System.Windows.Forms.TextBox();
		this.lbl51chR = new System.Windows.Forms.Label();
		this.tBox51chC = new System.Windows.Forms.TextBox();
		this.lbl51chLFE = new System.Windows.Forms.Label();
		this.tBox51chL = new System.Windows.Forms.TextBox();
		this.lbl51chLs = new System.Windows.Forms.Label();
		this.tBox51chLFE = new System.Windows.Forms.TextBox();
		this.lbl51chL = new System.Windows.Forms.Label();
		this.tBox51chR = new System.Windows.Forms.TextBox();
		this.lbl51chC = new System.Windows.Forms.Label();
		this.tBox51chRs = new System.Windows.Forms.TextBox();
		this.btn51chRs = new System.Windows.Forms.Button();
		this.btn51chC = new System.Windows.Forms.Button();
		this.btn51chR = new System.Windows.Forms.Button();
		this.btn51chL = new System.Windows.Forms.Button();
		this.btn51chLFE = new System.Windows.Forms.Button();
		this.btn51chLs = new System.Windows.Forms.Button();
		this.rbtn51ch = new System.Windows.Forms.RadioButton();
		this.rbtnMonoStereo = new System.Windows.Forms.RadioButton();
		this.rbtnNone = new System.Windows.Forms.RadioButton();
		this.panelMonoStereo.SuspendLayout();
		this.panel51ch.SuspendLayout();
		base.SuspendLayout();
		this.panelMonoStereo.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
		this.panelMonoStereo.Controls.Add(this.lblMonoStereo);
		this.panelMonoStereo.Controls.Add(this.btnMonoStereo);
		this.panelMonoStereo.Controls.Add(this.tboxMonoStereo);
		this.panelMonoStereo.Enabled = false;
		this.panelMonoStereo.Location = new System.Drawing.Point(0, 32);
		this.panelMonoStereo.Margin = new System.Windows.Forms.Padding(3, 4, 3, 4);
		this.panelMonoStereo.Name = "panelMonoStereo";
		this.panelMonoStereo.Size = new System.Drawing.Size(365, 44);
		this.panelMonoStereo.TabIndex = 29;
		this.lblMonoStereo.AutoSize = true;
		this.lblMonoStereo.Font = new System.Drawing.Font("Arial", 8.25f);
		this.lblMonoStereo.Location = new System.Drawing.Point(3, 2);
		this.lblMonoStereo.Name = "lblMonoStereo";
		this.lblMonoStereo.Size = new System.Drawing.Size(71, 14);
		this.lblMonoStereo.TabIndex = 8;
		this.lblMonoStereo.Text = "Mono/Stereo:";
		this.btnMonoStereo.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right;
		this.btnMonoStereo.Font = new System.Drawing.Font("Arial", 7f);
		this.btnMonoStereo.Location = new System.Drawing.Point(336, 18);
		this.btnMonoStereo.Margin = new System.Windows.Forms.Padding(3, 4, 3, 4);
		this.btnMonoStereo.Name = "btnMonoStereo";
		this.btnMonoStereo.Size = new System.Drawing.Size(24, 21);
		this.btnMonoStereo.TabIndex = 3;
		this.btnMonoStereo.Text = "...";
		this.btnMonoStereo.UseVisualStyleBackColor = true;
		this.btnMonoStereo.Click += new System.EventHandler(btnMonoStereo_Click);
		this.tboxMonoStereo.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
		this.tboxMonoStereo.Font = new System.Drawing.Font("Arial", 8.25f, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, 0);
		this.tboxMonoStereo.Location = new System.Drawing.Point(7, 19);
		this.tboxMonoStereo.Margin = new System.Windows.Forms.Padding(3, 4, 3, 4);
		this.tboxMonoStereo.Name = "tboxMonoStereo";
		this.tboxMonoStereo.Size = new System.Drawing.Size(325, 20);
		this.tboxMonoStereo.TabIndex = 2;
		this.tboxMonoStereo.Text = " *.wav";
		this.panel51ch.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
		this.panel51ch.Controls.Add(this.lbl51ch);
		this.panel51ch.Controls.Add(this.lbl51chRs);
		this.panel51ch.Controls.Add(this.tBox51chLs);
		this.panel51ch.Controls.Add(this.lbl51chR);
		this.panel51ch.Controls.Add(this.tBox51chC);
		this.panel51ch.Controls.Add(this.lbl51chLFE);
		this.panel51ch.Controls.Add(this.tBox51chL);
		this.panel51ch.Controls.Add(this.lbl51chLs);
		this.panel51ch.Controls.Add(this.tBox51chLFE);
		this.panel51ch.Controls.Add(this.lbl51chL);
		this.panel51ch.Controls.Add(this.tBox51chR);
		this.panel51ch.Controls.Add(this.lbl51chC);
		this.panel51ch.Controls.Add(this.tBox51chRs);
		this.panel51ch.Controls.Add(this.btn51chRs);
		this.panel51ch.Controls.Add(this.btn51chC);
		this.panel51ch.Controls.Add(this.btn51chR);
		this.panel51ch.Controls.Add(this.btn51chL);
		this.panel51ch.Controls.Add(this.btn51chLFE);
		this.panel51ch.Controls.Add(this.btn51chLs);
		this.panel51ch.Enabled = false;
		this.panel51ch.Location = new System.Drawing.Point(0, 88);
		this.panel51ch.Margin = new System.Windows.Forms.Padding(3, 4, 3, 4);
		this.panel51ch.Name = "panel51ch";
		this.panel51ch.Size = new System.Drawing.Size(365, 153);
		this.panel51ch.TabIndex = 28;
		this.lbl51ch.AutoSize = true;
		this.lbl51ch.Font = new System.Drawing.Font("Arial", 8.25f);
		this.lbl51ch.Location = new System.Drawing.Point(2, 4);
		this.lbl51ch.Name = "lbl51ch";
		this.lbl51ch.Size = new System.Drawing.Size(37, 14);
		this.lbl51ch.TabIndex = 9;
		this.lbl51ch.Text = "5.1ch:";
		this.lbl51chRs.AutoSize = true;
		this.lbl51chRs.Font = new System.Drawing.Font("Arial", 8.25f, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, 0);
		this.lbl51chRs.Location = new System.Drawing.Point(8, 125);
		this.lbl51chRs.Name = "lbl51chRs";
		this.lbl51chRs.Size = new System.Drawing.Size(20, 14);
		this.lbl51chRs.TabIndex = 27;
		this.lbl51chRs.Text = "Rs";
		this.tBox51chLs.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
		this.tBox51chLs.Location = new System.Drawing.Point(34, 101);
		this.tBox51chLs.Margin = new System.Windows.Forms.Padding(3, 4, 3, 4);
		this.tBox51chLs.Name = "tBox51chLs";
		this.tBox51chLs.Size = new System.Drawing.Size(298, 21);
		this.tBox51chLs.TabIndex = 12;
		this.lbl51chR.AutoSize = true;
		this.lbl51chR.Font = new System.Drawing.Font("Arial", 8.25f, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, 0);
		this.lbl51chR.Location = new System.Drawing.Point(14, 46);
		this.lbl51chR.Name = "lbl51chR";
		this.lbl51chR.Size = new System.Drawing.Size(14, 14);
		this.lbl51chR.TabIndex = 26;
		this.lbl51chR.Text = "R";
		this.tBox51chC.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
		this.tBox51chC.Location = new System.Drawing.Point(34, 61);
		this.tBox51chC.Margin = new System.Windows.Forms.Padding(3, 4, 3, 4);
		this.tBox51chC.Name = "tBox51chC";
		this.tBox51chC.Size = new System.Drawing.Size(298, 21);
		this.tBox51chC.TabIndex = 10;
		this.lbl51chLFE.AutoSize = true;
		this.lbl51chLFE.Font = new System.Drawing.Font("Arial", 8.25f, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, 0);
		this.lbl51chLFE.Location = new System.Drawing.Point(3, 85);
		this.lbl51chLFE.Name = "lbl51chLFE";
		this.lbl51chLFE.Size = new System.Drawing.Size(25, 14);
		this.lbl51chLFE.TabIndex = 25;
		this.lbl51chLFE.Text = "LFE";
		this.tBox51chL.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
		this.tBox51chL.Location = new System.Drawing.Point(34, 22);
		this.tBox51chL.Margin = new System.Windows.Forms.Padding(3, 4, 3, 4);
		this.tBox51chL.Name = "tBox51chL";
		this.tBox51chL.Size = new System.Drawing.Size(298, 21);
		this.tBox51chL.TabIndex = 11;
		this.lbl51chLs.AutoSize = true;
		this.lbl51chLs.Font = new System.Drawing.Font("Arial", 8.25f, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, 0);
		this.lbl51chLs.Location = new System.Drawing.Point(9, 105);
		this.lbl51chLs.Name = "lbl51chLs";
		this.lbl51chLs.Size = new System.Drawing.Size(19, 14);
		this.lbl51chLs.TabIndex = 24;
		this.lbl51chLs.Text = "Ls";
		this.tBox51chLFE.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
		this.tBox51chLFE.Location = new System.Drawing.Point(34, 81);
		this.tBox51chLFE.Margin = new System.Windows.Forms.Padding(3, 4, 3, 4);
		this.tBox51chLFE.Name = "tBox51chLFE";
		this.tBox51chLFE.Size = new System.Drawing.Size(298, 21);
		this.tBox51chLFE.TabIndex = 13;
		this.lbl51chL.AutoSize = true;
		this.lbl51chL.Font = new System.Drawing.Font("Arial", 8.25f, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, 0);
		this.lbl51chL.Location = new System.Drawing.Point(15, 26);
		this.lbl51chL.Name = "lbl51chL";
		this.lbl51chL.Size = new System.Drawing.Size(13, 14);
		this.lbl51chL.TabIndex = 23;
		this.lbl51chL.Text = "L";
		this.tBox51chR.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
		this.tBox51chR.Location = new System.Drawing.Point(34, 42);
		this.tBox51chR.Margin = new System.Windows.Forms.Padding(3, 4, 3, 4);
		this.tBox51chR.Name = "tBox51chR";
		this.tBox51chR.Size = new System.Drawing.Size(298, 21);
		this.tBox51chR.TabIndex = 14;
		this.lbl51chC.AutoSize = true;
		this.lbl51chC.Font = new System.Drawing.Font("Arial", 8.25f, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, 0);
		this.lbl51chC.Location = new System.Drawing.Point(14, 65);
		this.lbl51chC.Name = "lbl51chC";
		this.lbl51chC.Size = new System.Drawing.Size(14, 14);
		this.lbl51chC.TabIndex = 22;
		this.lbl51chC.Text = "C";
		this.tBox51chRs.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
		this.tBox51chRs.Location = new System.Drawing.Point(34, 121);
		this.tBox51chRs.Margin = new System.Windows.Forms.Padding(3, 4, 3, 4);
		this.tBox51chRs.Name = "tBox51chRs";
		this.tBox51chRs.Size = new System.Drawing.Size(298, 21);
		this.tBox51chRs.TabIndex = 15;
		this.btn51chRs.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right;
		this.btn51chRs.Font = new System.Drawing.Font("Arial", 7f);
		this.btn51chRs.Location = new System.Drawing.Point(336, 123);
		this.btn51chRs.Margin = new System.Windows.Forms.Padding(3, 4, 3, 4);
		this.btn51chRs.Name = "btn51chRs";
		this.btn51chRs.Size = new System.Drawing.Size(24, 21);
		this.btn51chRs.TabIndex = 21;
		this.btn51chRs.Text = "...";
		this.btn51chRs.UseVisualStyleBackColor = true;
		this.btn51chRs.Click += new System.EventHandler(btn51chRs_Click);
		this.btn51chC.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right;
		this.btn51chC.Font = new System.Drawing.Font("Arial", 7f);
		this.btn51chC.Location = new System.Drawing.Point(336, 62);
		this.btn51chC.Margin = new System.Windows.Forms.Padding(3, 4, 3, 4);
		this.btn51chC.Name = "btn51chC";
		this.btn51chC.Size = new System.Drawing.Size(24, 21);
		this.btn51chC.TabIndex = 16;
		this.btn51chC.Text = "...";
		this.btn51chC.UseVisualStyleBackColor = true;
		this.btn51chC.Click += new System.EventHandler(btn51chC_Click);
		this.btn51chR.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right;
		this.btn51chR.Font = new System.Drawing.Font("Arial", 7f);
		this.btn51chR.Location = new System.Drawing.Point(336, 42);
		this.btn51chR.Margin = new System.Windows.Forms.Padding(3, 4, 3, 4);
		this.btn51chR.Name = "btn51chR";
		this.btn51chR.Size = new System.Drawing.Size(24, 21);
		this.btn51chR.TabIndex = 20;
		this.btn51chR.Text = "...";
		this.btn51chR.UseVisualStyleBackColor = true;
		this.btn51chR.Click += new System.EventHandler(btn51chR_Click);
		this.btn51chL.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right;
		this.btn51chL.Font = new System.Drawing.Font("Arial", 7f);
		this.btn51chL.Location = new System.Drawing.Point(336, 22);
		this.btn51chL.Margin = new System.Windows.Forms.Padding(3, 4, 3, 4);
		this.btn51chL.Name = "btn51chL";
		this.btn51chL.Size = new System.Drawing.Size(24, 21);
		this.btn51chL.TabIndex = 17;
		this.btn51chL.Text = "...";
		this.btn51chL.UseVisualStyleBackColor = true;
		this.btn51chL.Click += new System.EventHandler(btn51chL_Click);
		this.btn51chLFE.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right;
		this.btn51chLFE.Font = new System.Drawing.Font("Arial", 7f);
		this.btn51chLFE.Location = new System.Drawing.Point(336, 82);
		this.btn51chLFE.Margin = new System.Windows.Forms.Padding(3, 4, 3, 4);
		this.btn51chLFE.Name = "btn51chLFE";
		this.btn51chLFE.Size = new System.Drawing.Size(24, 21);
		this.btn51chLFE.TabIndex = 19;
		this.btn51chLFE.Text = "...";
		this.btn51chLFE.UseVisualStyleBackColor = true;
		this.btn51chLFE.Click += new System.EventHandler(btn51chLFE_Click);
		this.btn51chLs.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right;
		this.btn51chLs.Font = new System.Drawing.Font("Arial", 7f);
		this.btn51chLs.Location = new System.Drawing.Point(336, 102);
		this.btn51chLs.Margin = new System.Windows.Forms.Padding(3, 4, 3, 4);
		this.btn51chLs.Name = "btn51chLs";
		this.btn51chLs.Size = new System.Drawing.Size(24, 21);
		this.btn51chLs.TabIndex = 18;
		this.btn51chLs.Text = "...";
		this.btn51chLs.UseVisualStyleBackColor = true;
		this.btn51chLs.Click += new System.EventHandler(btn51chLs_Click);
		this.rbtn51ch.AutoSize = true;
		this.rbtn51ch.Font = new System.Drawing.Font("Arial", 8.25f, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, 0);
		this.rbtn51ch.Location = new System.Drawing.Point(184, 6);
		this.rbtn51ch.Margin = new System.Windows.Forms.Padding(3, 4, 3, 4);
		this.rbtn51ch.Name = "rbtn51ch";
		this.rbtn51ch.Size = new System.Drawing.Size(113, 18);
		this.rbtn51ch.TabIndex = 7;
		this.rbtn51ch.Text = "5.1ch (Except Wii)";
		this.rbtn51ch.UseVisualStyleBackColor = true;
		this.rbtn51ch.CheckedChanged += new System.EventHandler(rbtn51ch_CheckedChanged);
		this.rbtnMonoStereo.AutoSize = true;
		this.rbtnMonoStereo.Font = new System.Drawing.Font("Arial", 8.25f, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, 0);
		this.rbtnMonoStereo.Location = new System.Drawing.Point(81, 6);
		this.rbtnMonoStereo.Margin = new System.Windows.Forms.Padding(3, 4, 3, 4);
		this.rbtnMonoStereo.Name = "rbtnMonoStereo";
		this.rbtnMonoStereo.Size = new System.Drawing.Size(86, 18);
		this.rbtnMonoStereo.TabIndex = 6;
		this.rbtnMonoStereo.Text = "Mono/Stereo";
		this.rbtnMonoStereo.UseVisualStyleBackColor = true;
		this.rbtnMonoStereo.CheckedChanged += new System.EventHandler(rbtnMonoStereo_CheckedChanged);
		this.rbtnNone.AutoSize = true;
		this.rbtnNone.Checked = true;
		this.rbtnNone.Font = new System.Drawing.Font("Arial", 8.25f, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, 0);
		this.rbtnNone.Location = new System.Drawing.Point(8, 6);
		this.rbtnNone.Margin = new System.Windows.Forms.Padding(3, 4, 3, 4);
		this.rbtnNone.Name = "rbtnNone";
		this.rbtnNone.Size = new System.Drawing.Size(50, 18);
		this.rbtnNone.TabIndex = 5;
		this.rbtnNone.TabStop = true;
		this.rbtnNone.Text = "None";
		this.rbtnNone.UseVisualStyleBackColor = true;
		this.rbtnNone.CheckedChanged += new System.EventHandler(rbtnNone_CheckedChanged);
		base.AutoScaleDimensions = new System.Drawing.SizeF(7f, 15f);
		base.Controls.Add(this.panelMonoStereo);
		base.Controls.Add(this.panel51ch);
		base.Controls.Add(this.rbtn51ch);
		base.Controls.Add(this.rbtnMonoStereo);
		base.Controls.Add(this.rbtnNone);
		this.Font = new System.Drawing.Font("Arial", 9f, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, 0);
		base.Margin = new System.Windows.Forms.Padding(3, 4, 3, 4);
		base.Name = "AudioSettingSubPanel";
		base.Size = new System.Drawing.Size(365, 241);
		this.panelMonoStereo.ResumeLayout(false);
		this.panelMonoStereo.PerformLayout();
		this.panel51ch.ResumeLayout(false);
		this.panel51ch.PerformLayout();
		base.ResumeLayout(false);
		base.PerformLayout();
	}
}
