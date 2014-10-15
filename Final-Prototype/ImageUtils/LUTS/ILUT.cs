using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ImageUtils
{
    public interface ILUT
    {
        bool SetupLookupTable(string p_pathtolutfile);
        void GetLookupValue(int pixelValue, out int r, out int g, out int b);
        Color[] GetLookupColors();

    }
}
