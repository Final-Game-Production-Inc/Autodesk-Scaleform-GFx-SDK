using System;
using System.ComponentModel;
using System.Drawing;
using System.IO;
using System.Windows.Forms;

namespace Sofdec2EncoderControl;

public class SubtitleSettingSubPanel : UserControl
{
	private IContainer components;

	private Button btnSubtitleFile;

	public TextBox tboxSubtitleFile;

	public SubtitleSettingSubPanel()
	{
		InitializeComponent();
	}

	public string GetSubtitleTextPath()
	{
		string text = tboxSubtitleFile.Text;
		if (!checkFilePath(text))
		{
			return string.Empty;
		}
		return text;
	}

	private void btnSubtitleFile_Click(object sender, EventArgs e)
	{
		OpenFileDialog openFileDialog = new OpenFileDialog();
		openFileDialog.Filter = "Subtitle Text Files(*.txt)|*.txt|All Files (*.*)|*.*";
		openFileDialog.RestoreDirectory = true;
		if (openFileDialog.ShowDialog() == DialogResult.OK)
		{
			tboxSubtitleFile.Text = openFileDialog.FileName;
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

	private void tboxSubtitleFile_TextChanged(object sender, EventArgs e)
	{
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
		this.btnSubtitleFile = new System.Windows.Forms.Button();
		this.tboxSubtitleFile = new System.Windows.Forms.TextBox();
		base.SuspendLayout();
		this.btnSubtitleFile.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right;
		this.btnSubtitleFile.Location = new System.Drawing.Point(208, 2);
		this.btnSubtitleFile.Margin = new System.Windows.Forms.Padding(3, 4, 3, 4);
		this.btnSubtitleFile.MaximumSize = new System.Drawing.Size(24, 22);
		this.btnSubtitleFile.Name = "btnSubtitleFile";
		this.btnSubtitleFile.Size = new System.Drawing.Size(24, 22);
		this.btnSubtitleFile.TabIndex = 10;
		this.btnSubtitleFile.Text = "...";
		this.btnSubtitleFile.UseVisualStyleBackColor = true;
		this.btnSubtitleFile.Click += new System.EventHandler(btnSubtitleFile_Click);
		this.tboxSubtitleFile.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
		this.tboxSubtitleFile.Font = new System.Drawing.Font("Arial", 8.25f, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, 0);
		this.tboxSubtitleFile.Location = new System.Drawing.Point(3, 3);
		this.tboxSubtitleFile.Margin = new System.Windows.Forms.Padding(3, 4, 3, 4);
		this.tboxSubtitleFile.Name = "tboxSubtitleFile";
		this.tboxSubtitleFile.Size = new System.Drawing.Size(199, 20);
		this.tboxSubtitleFile.TabIndex = 9;
		this.tboxSubtitleFile.Text = " *.txt";
		this.tboxSubtitleFile.TextChanged += new System.EventHandler(tboxSubtitleFile_TextChanged);
		base.AutoScaleDimensions = new System.Drawing.SizeF(6f, 13f);
		base.Controls.Add(this.btnSubtitleFile);
		base.Controls.Add(this.tboxSubtitleFile);
		base.Name = "SubtitleSettingSubPanel";
		base.Size = new System.Drawing.Size(235, 30);
		base.ResumeLayout(false);
		base.PerformLayout();
	}
}
