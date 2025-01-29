/* x,y in a 1 dimensional array. */
   double value;        /* The data in the new units. */
   /* A pointer to Grib_Data for ease of manipulation. */
   double *grib_Data = nullptr;
   sInt4 missCnt = 0;   /* Number of detected missing values. */
   uInt4 index;         /* Current index into Wx table. */
   float *ain = (float *) iain;
   uInt4 subNx;         /* The Nx dimension of the subgrid. */
   uInt4 subNy;         /* The Ny dimension of the subgrid. */

   subNx = stopX - startX + 1;
   subNy = stopY - startY + 1;

   myAssert (((!f_subGrid) && (subNx == Nx)) || (f_subGrid));
   myAssert (((!f_subGrid) && (subNy == Ny)) || (f_subGrid));

   if( subNy == 0 || subNx > UINT_MAX / subNy )
   {
       errSprintf ("Too large raster");
       *grib_DataLen = 0;
       *Grib_Data = nullptr;
       return;
   }

   const uInt4 subNxNy = subNx * subNy;
   if (subNxNy > *grib_DataLen) {

      if( subNxNy > 100 * 1024 * 1024 )
      {
          vsi_l_offset curPos = VSIFTellL(filePtr);
          VSIFSeekL(filePtr, 0, SEEK_END);
          vsi_l_offset fileSize = VSIFTellL(filePtr);
          VSIFSeekL(filePtr, curPos, SEEK_SET);      
          // allow a compression ratio of 1:1000
          if( subNxNy / 1000 > fileSize )
          {
            errSprintf ("ERROR: File too short\n");
            *grib_DataLen = 0;
            *Grib_Data = nullptr;
            return;
          }
      }

      *grib_DataLen = subNxNy;
      double* newData = (double *) realloc ((void *) (*Grib_Data),
                                       (*grib_DataLen) * sizeof (double));
      if( newData == nullptr )
      {
          errSprintf ("Memory allocation failed");
          free(*Grib_Data);
          *Grib_Data = nullptr;
          *grib_DataLen = 0;
          return;
      }
      *Grib_Data = newData;
   }
   grib_Data = *Grib_Data;

   /* Resolve possibility that the data is an integer or a float, find
    * max/min values, and do unit conversion. (see note 1) */
   if (scan == 64) {
      if (attrib->f_miss == 0) {
         ParseGridNoMiss (attrib, grib_Data, Nx, Ny, iain, unitM, unitB,
                          f_txtType, txt_dataLen, txt_f_valid, startX, startY, subNx, subNy);
      } else if (attrib->f_miss == 1) {
         ParseGridPrimMiss (attrib, grib_Data, Nx, Ny, iain, unitM, unitB,
                            &missCnt, f_txtType, txt_dataLen, txt_f_valid, startX, startY,
                            subNx, subNy);
      } else if (attrib->f_miss == 2) {
         ParseGridSecMiss (attrib, grib_Data, Nx, Ny, iain, unitM, unitB,
                           &missCnt, f_txtType, txt_dataLen, txt_f_valid, startX, startY, subNx,
                           subNy);
      }
   } else {
      /* Internally we use scan = 0100.  Scan is usually 0100 from the
       * unpacker library, but if scan is not, the following code converts
       * it.  We optimized the previous (scan 0100) case by calling a
       * dedicated procedure.  Here we don't since for scan != 0100, we
       * would_ need a different unpacker library, which is extremely
       * unlikely. */
      for (scanIndex = 0; scanIndex < (uInt4)nd2x3 && scanIndex < Nx * Ny; scanIndex++) {
         if (attrib->fieldType) {
            value = iain[scanIndex];
         } else {
            value = ain[scanIndex];
         }
         /* Make sure value is not a missing value when converting units, and
          * while computing max/min. */
         if ((attrib->f_miss == 0) ||
             ((attrib->f_miss == 1) && (value != attrib->missPri)) ||
             ((attrib->f_miss == 2) && (value != attrib->missPri) &&
              (value != attrib->missSec))) {
            /* Convert the units. */
            if (unitM == -10) {
               value = pow (10.0, value);
            } else {
               value = unitM * value + unitB;
            }
            /* Don't have to check if value became missing value, because we
             * can check if missing falls in the range of min/max.  If
             * missing does fall in that range we need to move missing. See
             * f_readjust */
            if (f_txtType) {
               index = (uInt4) value;
               if (index < txt_dataLen) {
                  if (txt_f_valid[index] == 1) {
                     txt_f_valid[index] = 2;
                  } else if (txt_f_valid[index] == 0) {
                     /* Table is not valid here so set value to missPri */
                     if (attrib->f_miss != 0) {
                        value = attrib->missPri;
                        missCnt++;
                     } else {
                        /* No missing value, so use index = WxType->dataLen */
                        /* No... set f_valid to 3 so we know we used this
                         * invalid element, then handle it in degrib2.c ::
                         * ReadGrib2Record() where we set it back to 0. */
                        txt_f_valid[index] = 3;
                     }
                  }
               }
            }
            if ((!f_txtType) ||
                ((attrib->f_miss == 0) || (value != attrib->missPri))) {
               if (attrib->f_maxmin) {
                  if (value < attrib->min) {
                     attrib->min = value;
                  } else if (value > attrib->max) {
                     attrib->max = value;
                  }
               } else {
                  attrib->min = attrib->max = value;
                  attrib->f_maxmin = 1;
               }
            }
         } else {
            missCnt++;
         }
         ScanIndex2XY (scanIndex, &x, &y, scan, Nx, Ny);
         /* ScanIndex returns value as if scan was 0100 */
         newIndex = (x - 1) + (y - 1) * Nx;
         grib_Data[newIndex] = value;
      }
   }

   /* Deal with possibility that unit conversion ended up with valid numbers
    * being interpreted as missing. */
   f_readjust = 0;
   xmissp = attrib->missPri;
   xmisss = attrib->missSec;
   if (attrib->f_maxmin) {
      if ((attrib->f_miss == 1) || (attrib->f_miss == 2)) {
         if ((attrib->missPri >= attrib->min) &&
             (attrib->missPri <= attrib->max)) {
            xmissp = attrib->max + 1;
            f_readjust = 1;
         }
         if (attrib->f_miss == 2) {
            if ((attrib->missSec >= attrib->min) &&
                (attrib->missSec <= attrib->max)) {
               xmisss = attrib->max + 2;
               f_readjust = 1;
            }
         }
      }
   }

   /* Walk through the grid, resetting the missing values, as determined by
    * the original grid. */
   if (f_readjust) {
      for (scanIndex = 0; scanIndex < (uInt4)nd2x3 && scanIndex < Nx * Ny; scanIndex++) {
         ScanIndex2XY (scanIndex, &x, &y, scan, Nx, Ny);
         /* ScanIndex returns value as if scan was 0100 */
         newIndex = (x - 1) + (y - 1) * Nx;
         if (attrib->fieldType) {
            value = iain[scanIndex];
         } else {
            value = ain[scanIndex];
         }
         if (value == attrib->missPri) {
            grib_Data[newIndex] = xmissp;
         } else if ((attrib->f_miss == 2) && (value == attrib->missSec)) {
            grib_Data[newIndex] = xmisss;
         }
      }
      attrib->missPri = xmissp;
      if (attrib->f_miss == 2) {
         attrib->missSec = xmisss;
      }
   }

   /* Resolve bitmap (if there is one) in the data. */
   if (ibitmap) {
      attrib->f_maxmin = 0;
      if ((attrib->f_miss != 1) && (attrib->f_miss != 2)) {
         missCnt = 0;
         /* Figure out a missing value. */
         xmissp = 9999;
#ifdef unreachable
         if (attrib->f_maxmin) {
            if ((xmissp <= attrib->max) && (xmissp >= attrib->min)) {
               xmissp = attrib->max + 1;
            }
         }
#endif
         /* embed the missing value. */
         for (scanIndex = 0; scanIndex < (uInt4)nd2x3 && scanIndex < Nx * Ny; scanIndex++) {
            ScanIndex2XY (scanIndex, &x, &y, scan, Nx, Ny);
            /* ScanIndex returns value as if scan was 0100 */
            newIndex = (x - 1) + (y - 1) * Nx;
            /* Corrected this on 5/10/2004 */
            if (ib[scanIndex] != 1) {
               grib_Data[newIndex] = xmissp;
               missCnt++;
            } else {
               if (!attrib->f_maxmin) {
                  attrib->f_maxmin = 1;
                  attrib->max = attrib->min = grib_Data[newIndex];
               } else {
                  if (attrib->max < grib_Data[newIndex])
                     attrib->max = grib_Data[newIndex];
                  if (attrib->min > grib_Data[newIndex])
                     attrib->min = grib_Data[newIndex];
               }
            }
         }
         attrib->f_miss = 1;
         attrib->missPri = xmissp;
      }
      if (!attrib->f_maxmin) {
         attrib->f_maxmin = 1;
         attrib->max = attrib->min = xmissp;
      }
   }