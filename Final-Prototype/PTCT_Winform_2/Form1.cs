using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using PTCT_VTK_BRIDGE;

namespace PTCT_Winform_2
{
    public partial class Form1 : Form
    {
        private MPR_UI_Interface m_bHandle;
        private ImageControl ctImage;
        public Form1()
        {
            InitializeComponent();
            this.m_bHandle = PTCT_VTK_BRIDGE.MPR_UI_Interface.GetHandle();
            
        }

        private void button1_Click(object sender, EventArgs e)
        {
            string _ctImage = @"D:\DicomDataSet\PTCT\Dataset1\1.3.12.2.1107.5.1.4.1007.30000012032001413290600020180\1.3.12.2.1107.5.1.4.1007.30000012032001413290600020493.dcm";
            string _ptImage = @"D:\DicomDataSet\PTCT\Dataset1\1.3.12.2.1107.5.1.4.1007.30000012032001433800000011963\1.3.12.2.1107.5.1.4.1007.30000012032001433800000011650.dcm";
            this.m_bHandle.LoadImages(_ctImage, _ptImage);

            tableLayoutPanel1.SuspendLayout();

            ctImage = new ImageControl(ViewType.CT);
            ctImage.Dock = DockStyle.Fill;
            tableLayoutPanel1.Controls.Add(ctImage, 0, 0);

            ptImage = new ImageControl(ViewType.PT);
            ptImage.Dock = DockStyle.Fill;
            tableLayoutPanel1.Controls.Add(ptImage, 1, 0);

            fusedImage = new ImageControl(ViewType.FUSED);
            fusedImage.Dock = DockStyle.Fill;
            tableLayoutPanel1.Controls.Add(fusedImage, 2, 0);

            tableLayoutPanel1.ResumeLayout();


        }

        public ImageControl ptImage { get; set; }

        public ImageControl fusedImage { get; set; }
    }
}
