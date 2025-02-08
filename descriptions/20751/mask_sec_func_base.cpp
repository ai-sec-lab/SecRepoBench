void ParseGrid (VSILFILE *fp, gridAttribType *attrib, double **Grib_Data,
                uInt4 *grib_DataLen, uInt4 Nx, uInt4 Ny, int scan,
                sInt4 nd2x3, sInt4 *iain, sInt4 ibitmap, sInt4 *ib, double unitM,
                double unitB, uChar f_txtType, uInt4 txt_dataLen,
                uChar *txt_f_valid,
                CPL_UNUSED uChar f_subGrid,
                int startX, int startY, int stopX, int stopY)
{
   double xmissp;       /* computed missing value needed for ibitmap = 1,
                         * Also used if unit conversion causes confusion
                         * over_ missing values. */
   double xmisss;       /* Used if unit conversion causes confusion over
                         * missing values. */
   uChar f_readjust;    /* True if unit conversion caused confusion over
                         * missing values. */
   uInt4 scanIndex;     /* Where we are in the original grid. */
   sInt4 x, y;          /* Where we are in a grid of scan value 0100 */
   uInt4 newIndex;      // <MASK>
   attrib->numMiss = missCnt;
}