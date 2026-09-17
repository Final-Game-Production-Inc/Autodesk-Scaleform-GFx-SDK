using System.ComponentModel;
using System.Drawing;
using System.Windows.Forms;

namespace Sofdec2EncoderControl;

public class VideoSubPanel : UserControl
{
	private IContainer components;

	private Panel videoPanel;

	private Label lblVideoTime;

	private TrackBar trackVideo;

	public VideoSubPanel()
	{
		InitializeComponent();
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
		this.videoPanel = new System.Windows.Forms.Panel();
		this.lblVideoTime = new System.Windows.Forms.Label();
		this.trackVideo = new System.Windows.Forms.TrackBar();
		((System.ComponentModel.ISupportInitialize)this.trackVideo).BeginInit();
		base.SuspendLayout();
		this.videoPanel.Anchor = System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
		this.videoPanel.BackColor = System.Drawing.Color.Black;
		this.videoPanel.Location = new System.Drawing.Point(14, 10);
		this.videoPanel.Name = "videoPanel";
		this.videoPanel.Size = new System.Drawing.Size(512, 354);
		this.videoPanel.TabIndex = 1;
		this.lblVideoTime.AutoSize = true;
		this.lblVideoTime.Location = new System.Drawing.Point(3, 394);
		this.lblVideoTime.Name = "lblVideoTime";
		this.lblVideoTime.Size = new System.Drawing.Size(64, 13);
		this.lblVideoTime.TabIndex = 2;
		this.lblVideoTime.Text = "00:00:00:00";
		this.trackVideo.Anchor = System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right;
		this.trackVideo.LargeChange = 1;
		this.trackVideo.Location = new System.Drawing.Point(12, 366);
		this.trackVideo.Maximum = 100;
		this.trackVideo.Name = "trackVideo";
		this.trackVideo.Size = new System.Drawing.Size(512, 45);
		this.trackVideo.TabIndex = 3;
		this.trackVideo.TickStyle = System.Windows.Forms.TickStyle.None;
		base.AutoScaleDimensions = new System.Drawing.SizeF(6f, 13f);
		base.Controls.Add(this.lblVideoTime);
		base.Controls.Add(this.videoPanel);
		base.Controls.Add(this.trackVideo);
		base.Name = "VideoSubPanel";
		base.Size = new System.Drawing.Size(536, 412);
		((System.ComponentModel.ISupportInitialize)this.trackVideo).EndInit();
		base.ResumeLayout(false);
		base.PerformLayout();
	}
}
