using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using ImageUtils;
namespace EventDelegate_BRIDGE
{
    /// <summary>
    /// Event delegate bridge
    /// </summary>
    public class EDB
    {
        #region Constructor
        private static EDB instance;
        private EDB() { }
        public static EDB Instance
        {
            get
            {
                if (instance == null)
                {
                    instance = new EDB();
                }
                return instance;
            }
        }
        #endregion

        #region Update image
        public delegate void UpdatePTCTImage(int axis, BitmapWrapper ct_bmp, BitmapWrapper pt_bmp, double pt_pos_x, double pt_pos_y);
        private UpdatePTCTImage m_updatePTCTImage;
        
        /// <summary>
        /// add/remove event handler
        /// </summary>
        public event UpdatePTCTImage EVT_UpdatePTCTImage
        {
            [MethodImpl(MethodImplOptions.Synchronized)]
            add
            {
                m_updatePTCTImage = (UpdatePTCTImage)Delegate.Combine(m_updatePTCTImage, value);
            }
            [MethodImpl(MethodImplOptions.Synchronized)]
            remove
            {
                m_updatePTCTImage = (UpdatePTCTImage)Delegate.Remove(m_updatePTCTImage, value);
            }
        }

        /// <summary>
        /// Raise Update image event.
        /// </summary>
        /// <param name="axis">Slicer axis</param>
        /// <param name="ct_bmp">CT Bitmap</param>
        /// <param name="pt_bmp">PT Bitmap</param>
        /// <param name="pt_pos_x">PT X:Pos</param>
        /// <param name="pt_pos_y">PT Y:Pos</param>
        public void RaiseUpdatePTCTImage(int axis, BitmapWrapper ct_bmp, BitmapWrapper pt_bmp, double pt_pos_x, double pt_pos_y)
        {
            if (m_updatePTCTImage == null) return;
            var receivers = m_updatePTCTImage.GetInvocationList();

            foreach (UpdatePTCTImage receiver in receivers)
            {
                receiver.BeginInvoke(axis, ct_bmp, pt_bmp, pt_pos_x, pt_pos_y, null, null);
            }
        }
        #endregion
    }
}
