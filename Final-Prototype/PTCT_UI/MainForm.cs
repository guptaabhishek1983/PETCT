using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using ImageUtils;
using MPR_UI.Properties;
using PTCT_VTK_BRIDGE;

namespace MPR_UI
{
    [System.Runtime.InteropServices.GuidAttribute("063C04C5-9EA1-45A2-AE52-2AEB854437CE")]
    public partial class MainForm : Form
    {
        private ImageControl axialImage;
        private ImageControl coronalImage;
        private ImageControl sagittalImage;
        public MainForm()
        {
            InitializeComponent();
            this.Load += Form1_Load;
        }

        void Form1_Load(object sender, EventArgs e)
        {
            
            tableLayoutPanel1.SuspendLayout();

            axialImage = new ImageControl(Axis.AxialAxis);
            axialImage.Dock = DockStyle.Fill;
            
            axialPanel.Controls.Add(axialImage);
      
            //tableLayoutPanel1.Controls.Add(axialImage, 0, 0);

            sagittalImage = new ImageControl(Axis.SagittalAxis);
            sagittalImage.Dock = DockStyle.Fill;
            sagittalPanel.Controls.Add(sagittalImage);
            //tableLayoutPanel1.Controls.Add(sagittalImage, 1, 0);

            coronalImage = new ImageControl(Axis.CoronalAxis);
            coronalImage.Dock = DockStyle.Fill;
            coronalPanel.Controls.Add(coronalImage);
            //tableLayoutPanel1.Controls.Add(coronalImage, 2, 0);
            
            tableLayoutPanel1.ResumeLayout();
            MessageBox.Show("Done");
        }

        private void MainForm_Resize(object sender, EventArgs e)
        {
            
        }

        private void button1_Click(object sender, EventArgs e)
        {
            button1.Enabled = false;
            MPR_UI_Interface ui_interface = MPR_UI_Interface.GetHandle();
            ui_interface.InitMPR(Settings.Default.CT_DICOM_DIR);
            ui_interface.Init_PT_MPR(Settings.Default.PT_DICOM_DIR);
            axialImage.Init();
            sagittalImage.Init();
            coronalImage.Init();
            //sagittalImage.InitScrollBarAndLoadImage();
            //coronalImage.InitScrollBarAndLoadImage();
            MessageBox.Show("CT Loaded");
        }

        private void tableLayoutPanel1_Paint(object sender, PaintEventArgs e)
        {

        }

        private void button2_Click(object sender, EventArgs e)
        {
            //button2.Enabled = false;
            //MPR_UI_Interface ui_interface = MPR_UI_Interface.GetHandle();
            //ui_interface.Init_PT_MPR(Settings.Default.PT_DICOM_DIR);
            //axialImage.LoadPTImage();
            //sagittalImage.LoadPTImage();
            //coronalImage.LoadPTImage();
            //MessageBox.Show("PT Loaded");
        }
    }
}
