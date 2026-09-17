using ScaleformVideoEncoder;
using ScaleformVideoEncoder.Design;
using System;
using System.Windows.Forms;

namespace CriMvSimpleEncoder
{
    internal static class Program
    {
        private const string appName = "ScaleformVideoEncoder Reborn";

        private const string consoleEncoderPath = "sofdec2enc.exe";

        private const string videoPlayerPath = "GFxMediaPlayer.exe";

        private const string videoPlayerSwfPath = "VideoPlayer.swf";

        public static void ShowErrorMessage(string msg, bool isError)
        {
            string caption = "Scaleform Video Encoder : Error";
            MessageBoxIcon icon = (isError ? MessageBoxIcon.Hand : MessageBoxIcon.Exclamation);
            MessageBox.Show(msg, caption, MessageBoxButtons.OK, icon);
        }

        [STAThread]
        private static void Main()
        {
            Sofdec2Encoder encoder = new Sofdec2Encoder("sofdec2enc.exe", "GFxMediaPlayer.exe", "VideoPlayer.swf", "Scaleform Video Encoder");
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new VideoEncoderForm(encoder));
        }
    }
}
