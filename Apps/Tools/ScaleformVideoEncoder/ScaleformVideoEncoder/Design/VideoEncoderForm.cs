using ScaleformVideoEncoder.Design;

namespace ScaleformVideoEncoder
{
    public partial class VideoEncoderForm : Form
    {
        public VideoEncoderForm()
        {
            InitializeComponent();
        }

        public VideoEncoderForm(Sofdec2Encoder encoder)
        {
            InitializeComponent();
            this.encoder = encoder;

            encoder.EncodingDataReceived += encodeParamControl.AppendEncodingLogReceived;
            encoder.EncodingErrorReceived += encodeParamControl.AppendEncodingErrorReceived;
            encoder.EncodingProgressChenged += encodeParamControl.EncodingProgressChanged;
            encoder.EncodingExited += encodeParamControl.EncodingExited;
        }

    }
}
