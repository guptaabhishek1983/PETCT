using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml;

namespace ImageUtils
{
    public class PET:ILUT
    {
        Dictionary<int, Color> lut = new Dictionary<int, Color>();
        #region ILUT Members

        public bool SetupLookupTable(string p_pathtolutfile)
        {

            XmlDocument xmlDoc = new XmlDocument();
            xmlDoc.Load(p_pathtolutfile);
            XmlNodeList componentList = xmlDoc.SelectNodes("LUT/Component");
            List<int> blue = new List<int>();
            List<int> red = new List<int>();
            List<int> green = new List<int>();

            for (int i = 0; i < componentList.Count; i++)
            {
                XmlNode node = componentList.Item(i);
                XmlAttributeCollection attrs = node.Attributes;
                String comp = attrs.Item(0).InnerText;
                String delimStr = ",";
                char[] delimiter = delimStr.ToCharArray();
                String[] component;
                component = node.InnerText.Split(delimiter);

                if (String.Compare(comp, "blue") == 0)
                {
                    for (int b_i = 0; b_i < component.Length; b_i++)
                    {
                        blue.Add(Convert.ToInt16(component[b_i]));
                    }
                }
                if (String.Compare(comp, "green") == 0)
                {
                    for (int g_i = 0; g_i < component.Length; g_i++)
                    {
                        green.Add(Convert.ToInt16(component[g_i]));
                    }
                }
                if (String.Compare(comp, "red") == 0)
                {
                    for (int r_i = 0; r_i < component.Length; r_i++)
                    {
                        red.Add(Convert.ToInt16(component[r_i]));
                    }
                }
            }

            if ((blue.Count == red.Count) && (blue.Count== green.Count))
            {
                for(int i=0; i<blue.Count; i++)
                {
                    lut.Add(Convert.ToInt16(i), Color.FromArgb(red[i], green[i], blue[i]));
                }
            }
            else
            {
                return false;
            }
            return true;
        }

        

        public void GetLookupValue(int pixelValue, out int r, out int g, out int b)
        {
            r = Color.Transparent.R;
            g = Color.Transparent.G;
            b = Color.Transparent.B;
            if (lut.ContainsKey(pixelValue))
            {
                r = lut[pixelValue].R;
                g = lut[pixelValue].G;
                b = lut[pixelValue].B;
            }
        }

        #endregion
    }
}
