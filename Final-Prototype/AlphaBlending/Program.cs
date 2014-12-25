using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
using System.Linq;
using System.Text;

namespace AlphaBlending
{
    class Program
    {
        static void Main(string[] args)
        {
            byte alpha = 255;
            string ct_image_path = "D:\\CT.jpg";
            string pt_image_path = "D:\\PT.jpg";

            string outpath = "D:\\Fused_0.90.jpg";
            // for the matrix the range is 0.0 - 1.0
            float alphaNorm = 1.0F;//(float)alpha / 255.0F;
            using (Bitmap ct_image = (Bitmap)Bitmap.FromFile(ct_image_path))
            {
                using (Bitmap pt_image = (Bitmap)Bitmap.FromFile(pt_image_path))
                {
                    // just change the alpha
                    ColorMatrix matrix = new ColorMatrix(new float[][]{
                    new float[] {1F, 0, 0, 0, 0},
                    new float[] {0, 1F, 0, 0, 0},
                    new float[] {0, 0, 1F, 0, 0},
                    new float[] {0, 0, 0, alphaNorm, 0},
                    new float[] {0, 0, 0, 0, 1F}});

                    ImageAttributes imageAttributes = new ImageAttributes();
                    imageAttributes.SetColorMatrix(matrix);
                    //Bitmap result = new Bitmap(image1.Width, image1.Height);
                    Rectangle pt_rect = new Rectangle(0, 0, pt_image.Width, pt_image.Height);
                    Rectangle ct_rect = new Rectangle(0, 0, ct_image.Width, ct_image.Height);


                    Bitmap tempBitmap = new Bitmap(ct_image.Width, ct_image.Height);
                    // From this bitmap, the graphics can be obtained, because it has the right PixelFormat
                    using (Graphics g = Graphics.FromImage(tempBitmap))
                    {
                        // Draw the original bitmap onto the graphics of the new bitmap
                        g.DrawImage(ct_image, 0, 0);
                    }


                    using (Graphics g = Graphics.FromImage(tempBitmap))
                    {
                        Point center_point_pt = new Point(pt_rect.Left + pt_rect.Width / 2,
                    pt_rect.Top + pt_rect.Height / 2);

                        Point center_point_ct = new Point(ct_rect.Left + ct_rect.Width / 2,
                    ct_rect.Top + ct_rect.Height / 2);


                        Point displacement = new Point(center_point_ct.X - center_point_pt.X, center_point_ct.Y - center_point_pt.Y);

                        pt_rect.Offset(displacement);
                        

                        g.CompositingMode = CompositingMode.SourceOver;
                        g.CompositingQuality = CompositingQuality.HighQuality;
                        g.InterpolationMode = InterpolationMode.HighQualityBicubic;
                        g.DrawImage(pt_image,
                           pt_rect,
                            0,
                            0,
                            pt_image.Width,
                            pt_image.Height,
                            GraphicsUnit.Pixel,
                            imageAttributes);
                        
                    }
                    tempBitmap.Save(outpath);

                }
            }
        }
    }
}
