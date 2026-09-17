using ScaleformVideoEncoder.Design;
using Sofdec2EncoderControl;

namespace ScaleformVideoEncoder
{
    partial class VideoEncoderForm
    {

        private System.ComponentModel.IContainer components = null;

        private Sofdec2Encoder encoder;

        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(VideoEncoderForm));
            ScaleformLogo = new PictureBox();
            CriLogo = new PictureBox();
            encodeParamControl = new ParametersSettingPanel();
            ((System.ComponentModel.ISupportInitialize)ScaleformLogo).BeginInit();
            ((System.ComponentModel.ISupportInitialize)CriLogo).BeginInit();
            SuspendLayout();
            // 
            // ScaleformLogo
            // 
            ScaleformLogo.BackColor = Color.Black;
            ScaleformLogo.Dock = DockStyle.Top;
            ScaleformLogo.ErrorImage = null;
            ScaleformLogo.Image = (Image)resources.GetObject("ScaleformLogo.Image");
            ScaleformLogo.InitialImage = null;
            ScaleformLogo.Location = new Point(0, 0);
            ScaleformLogo.MinimumSize = new Size(0, 60);
            ScaleformLogo.Name = "ScaleformLogo";
            ScaleformLogo.Size = new Size(840, 60);
            ScaleformLogo.TabIndex = 0;
            ScaleformLogo.TabStop = false;
            // 
            // CriLogo
            // 
            CriLogo.Anchor = AnchorStyles.Top | AnchorStyles.Right;
            CriLogo.BackColor = Color.Black;
            CriLogo.ErrorImage = null;
            CriLogo.Image = (Image)resources.GetObject("CriLogo.Image");
            CriLogo.InitialImage = null;
            CriLogo.Location = new Point(761, -2);
            CriLogo.Margin = new Padding(0);
            CriLogo.MinimumSize = new Size(0, 60);
            CriLogo.Name = "CriLogo";
            CriLogo.Size = new Size(64, 60);
            CriLogo.TabIndex = 1;
            CriLogo.TabStop = false;
            // 
            // encodeParamControl
            // 
            encodeParamControl.Dock = DockStyle.Fill;
            encodeParamControl.Location = new Point(0, 60);
            encodeParamControl.Name = "encodeParamControl";
            encodeParamControl.Size = new Size(840, 726);
            encodeParamControl.TabIndex = 2;
            encodeParamControl.EncodingRequestedEvent += mainForm_EncodingRequested;
            encodeParamControl.CancelRequestedEvent += mainForm_CancelEncodingRequested;
            encodeParamControl.PreviewRequestedEvent += mainForm_PreviewRequested;
            encodeParamControl.LoadRequestedEvent += mainForm_LoadRequested;
            // 
            // VideoEncoderForm
            // 
            AutoScaleDimensions = new SizeF(7F, 15F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(840, 786);
            Controls.Add(encodeParamControl);
            Controls.Add(CriLogo);
            Controls.Add(ScaleformLogo);
            Icon = (Icon)resources.GetObject("$this.Icon");
            MinimumSize = new Size(800, 600);
            Name = "VideoEncoderForm";
            Text = "ScaleformVideoEncoder Reborn";
            ((System.ComponentModel.ISupportInitialize)ScaleformLogo).EndInit();
            ((System.ComponentModel.ISupportInitialize)CriLogo).EndInit();
            ResumeLayout(false);
        }

        private void mainForm_EncodingRequested(object sender, ParametersSettingPanel.EncodingRequestedEventArgs e)
        {
            encoder.StartEncode(e.EnvParam);
        }

        private void mainForm_CancelEncodingRequested(object sender, EventArgs e)
        {
            encoder.CancelEncode();
        }

        private void mainForm_PreviewRequested(object sender, ParametersSettingPanel.PreviewRequestedEventArgs e)
        {
            encoder.Preview(e.FileName, e.ExtendedPlayer);
        }

        private void mainForm_LoadRequested(object sender, ParametersSettingPanel.LoadRequestedEventArgs e)
        {
            encoder.Load(e.FileName);
        }

        private void encodeParamControl_Load(object sender, EventArgs e)
        {
        }

        #endregion

        private PictureBox ScaleformLogo;
        private PictureBox CriLogo;
        private ParametersSettingPanel encodeParamControl;
    }
}
