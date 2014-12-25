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
using System.Threading;
using System.Reflection;

namespace MPR_UI
{
    public partial class ImageControl : UserControl
    {
        private Axis m_axis;
        private ImagePanel2 m_imagePanel;
        private MPR_UI_Interface m_UIInterface;
        private double m_position;
        private int m_index;
        private bool m_scrollbarInit = false;

        public ImageControl()
        {
            InitializeComponent();
        }
        
        public ImageControl(Axis axis)
        {
            InitializeComponent();
            this.m_axis = axis;
            this.m_scrollbarInit = false;
            this.m_imagePanel = new ImagePanel2();
            this.m_imagePanel.Dock = DockStyle.Fill;
            this.m_imagePanel.EVT_MPRCursorTranslated += TranslateMPRCursor;
            this.m_imagePanel.EVT_RaisePixelIntensity += RaisePixelIntensity;
            this.panel1.Controls.Add(this.m_imagePanel);
            m_UIInterface = MPR_UI_Interface.GetHandle();
            EventDelegate_BRIDGE.EDB.Instance.EVT_UpdatePTCTImage += Handle_UpdatePTCTImage;
            EventDelegate_BRIDGE.EDB.Instance.EVT_UpdateCursorPos+=Handle_UpdateCursorPos;
            EventDelegate_BRIDGE.EDB.Instance.EVT_PixelIntensityAndSUV+=PixelIntensityAndSUV;
        }

        private void PixelIntensityAndSUV(int axis, int pt_pos_x, int pt_pos_y, double suv, int hu)
        {
            PET_MouseLocation = new Point(pt_pos_x, pt_pos_y);
            SUV = suv;
            HU = hu;
        }

        private void RaisePixelIntensity(Point p)
        {
            if (m_axis == Axis.SagittalAxis)
            {
                int val = this.m_UIInterface.GetPixelIntensity((int)m_axis, p.X, p.Y);
            }
                //RectangleF rect = new RectangleF(0, 0, 200, 200);
                //PointF rect1_origin = new PointF(m_imagePanel.imageRect.Left, m_imagePanel.imageRect.Top);
                //PointF rect2_origin = new PointF(0,0);
            
        }

        private void Handle_UpdatePTCTImage(int axis, BitmapWrapper ct_bmp, BitmapWrapper pt_bmp, double pt_pos_x, double pt_pos_y, double ct_pos, double pt_pos)
        {
            if (this.InvokeRequired)
            {
                this.Invoke(new MethodInvoker(delegate() { Handle_UpdatePTCTImage(axis, ct_bmp, pt_bmp, pt_pos_x, pt_pos_y, ct_pos, pt_pos); }));

            }
            else
            {
                try
                {
                    if (axis == (int)this.m_axis)
                    {
                        InitScrollBar();
                        
                        this.Position = ct_pos;
                        this.PetSlicerPosition = pt_pos;
                        this.m_imagePanel.StoreBitmap = ct_bmp.StoredBitmap;
                        // set PT bmp
                        this.m_imagePanel.PetPosition = new PointF((float)(pt_pos_x), (float)(pt_pos_y));
                        this.m_imagePanel.PET_StoreBitmap = pt_bmp.StoredBitmap;

                        Invalidate();
                        Update();
                    }
                }
                catch (Exception ex)
                {
                    MessageBox.Show(ex.ToString());
                }
            }
        }

        private void Handle_UpdateCursorPos(int axis, double pos_x, double pos_y)
        {
            if(axis != (int)m_axis)
            {
                return;
            }
            if (this.InvokeRequired)
            {
                this.Invoke(new MethodInvoker(delegate() { Handle_UpdateCursorPos(axis,pos_x, pos_y); }));
            }
            else
            {
                PointF cursorPoint = new PointF((float)pos_x, (float)pos_y);
                switch ((Axis)axis)
                {
                    case Axis.AxialAxis:
                        {
                            this.m_imagePanel.SetCursorPositionX_Axis(cursorPoint, Axis.SagittalAxis);
                            this.m_imagePanel.SetCursorPositionY_Axis(cursorPoint, Axis.CoronalAxis);
                        }
                        break;

                    case Axis.CoronalAxis:
                        {
                            this.m_imagePanel.SetCursorPositionX_Axis(cursorPoint, Axis.SagittalAxis);
                            this.m_imagePanel.SetCursorPositionY_Axis(cursorPoint, Axis.AxialAxis);
                        }
                        break;

                    case Axis.SagittalAxis:
                        {
                            this.m_imagePanel.SetCursorPositionX_Axis(cursorPoint, Axis.CoronalAxis);
                            this.m_imagePanel.SetCursorPositionY_Axis(cursorPoint, Axis.AxialAxis);
                        }
                        break;

                }
                Invalidate();
                Update();
            }
        }

       

        private void TranslateMPRCursor(Point p)
        {
            m_UIInterface.UpdateSlicerPosition((int)this.m_axis, (float)p.X, (float)p.Y);
            UpdateCursorPosition();
            Invalidate();
            Update();
        }

        private void UpdateCursorPosition()
        {
            //throw new NotImplementedException();
        }

        internal void InitScrollBar()
        {
            if (!m_scrollbarInit)
            {
                MPR_UI_Interface.WriteLog("Initializing scroll bar");
                this.scrollBar.Maximum = m_UIInterface.GetNumberOfImages((int)this.m_axis);
                this.scrollBar.Minimum = 0;
                this.scrollBar.SmallChange = 1;
                this.scrollBar.LargeChange = 1;
               ScrollBarCurrentVal = this.scrollBar.Value = m_UIInterface.GetCurrentImageIndex((int)this.m_axis);

                // Init scroll bar event
                this.scrollBar.ValueChanged += scrollBar_ValueChanged;


                // ORIENTATION MARKERS
                OrientationMarkerLeft = m_UIInterface.GetOrientationMarkerLeft((int)this.m_axis);
                OrientationMarkerRight = m_UIInterface.GetOrientationMarkerRight((int)this.m_axis);
                OrientationMarkerTop = m_UIInterface.GetOrientationMarkerTop((int)this.m_axis);
                OrientationMarkerBottom = m_UIInterface.GetOrientationMarkerBottom((int)this.m_axis);

                this.m_UIInterface.RaiseSlicerPositionUpdate((int)this.m_axis);
                m_scrollbarInit = true;
            }
            
        }

        
        void scrollBar_ValueChanged(object sender, EventArgs e)
        {

            m_UIInterface.Scroll((int)this.m_axis, ScrollBarCurrentVal - scrollBar.Value);
            ScrollBarCurrentVal = scrollBar.Value;
        }


        
        
        internal int GetScrollbarValue()
        {
            return scrollBar.Value;
        }

        internal double Position
        {
            get { return m_position; }
            set { m_position = value; }
        }

        internal int Index
        {
            get { return m_index; }
            set { m_index = value; }
        }

        internal string OrientationMarkerLeft { set; get; }
        internal string OrientationMarkerRight { set; get; }
        internal string OrientationMarkerBottom { set; get; }
        internal string OrientationMarkerTop { set; get; }

        internal void UpdateCursor(System.Windows.Forms.Cursor cursor)
        {
            this.Cursor = cursor;
        }

        internal void Init()
        {
            this.m_UIInterface.InitDisplay((int)this.m_axis);
        }

        public int ScrollBarCurrentVal { get; set; }

        public double PetSlicerPosition { get; set; }

        public Point PET_MouseLocation { get; set; }

        public double SUV { get; set; }

        public int HU { get; set; }
    }
}
