using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using PTCT_VTK_BRIDGE;
using ImageUtils;

namespace PTCT_Winform_2
{
    public partial class ImageControl : UserControl
    {
        private ViewType viewType;
        private MPR_UI_Interface m_bHandle;
        private ImagePanel3 img_panel;
        private ImageControl()
        {
            InitializeComponent();
        }

        public ImageControl(ViewType p_viewType)
        {
            InitializeComponent();
            this.viewType = p_viewType;
            this.m_bHandle = MPR_UI_Interface.GetHandle();
            this.Load += ImageControl_Load;
        }

        void ImageControl_Load(object sender, EventArgs e)
        {
            switch(viewType)
            {
                case ViewType.CT:
                    {
                        BitmapWrapper ct_bmp = this.m_bHandle.GetCTDisplay();
                        img_panel = new ImagePanel3();
                        img_panel.Dock = DockStyle.Fill;
                        this.panel1.Controls.Add(img_panel);
                        img_panel.StoreBitmap = ct_bmp.StoredBitmap;

                    }
                    break;

                case ViewType.PT:
                    {
                        BitmapWrapper pt_bmp = this.m_bHandle.GetPTDisplay();
                        img_panel = new ImagePanel3();
                        img_panel.Dock = DockStyle.Fill; 
                        this.panel1.Controls.Add(img_panel);
                        img_panel.StoreBitmap = pt_bmp.StoredBitmap;

                        //BitmapWrapper pt_lut_bmp = this.m_bHandle.GetPT_LUTDisplay();
                        //pt_lut_bmp.Resize((int)(pt_lut_bmp.StoredBitmap.Width * img_panel.currentZoomFactor), (int)(pt_lut_bmp.StoredBitmap.Height * img_panel.currentZoomFactor));
                        //img_panel.PET_StoreBitmap = pt_lut_bmp.StoredBitmap;

                    }
                    break;

                case ViewType.FUSED:
                    {
                        BitmapWrapper ct_bmp = this.m_bHandle.GetCTDisplay();
                        img_panel = new ImagePanel3();
                        img_panel.Dock = DockStyle.Fill; 
                        this.panel1.Controls.Add(img_panel);

                        img_panel.StoreBitmap = ct_bmp.StoredBitmap;
                        
                        BitmapWrapper pt_bmp = this.m_bHandle.GetPT_LUTDisplay();
                        //pt_bmp.Resize((int)(pt_bmp.StoredBitmap.Width * img_panel.currentZoomFactor), (int)(pt_bmp.StoredBitmap.Height * img_panel.currentZoomFactor));
                        img_panel.PetPosition = new PointF((float)(this.m_bHandle.GetTranslateX()), (float)(this.m_bHandle.GetTranslateY()));
                        img_panel.PET_StoreBitmap = pt_bmp.StoredBitmap;


                        
                    }
                    break;

            }
        }
    }
}
