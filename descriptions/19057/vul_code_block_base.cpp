if ((templat != GS4_ANALYSIS) && (templat != GS4_ENSEMBLE)
       && (templat != GS4_DERIVED)
       && (templat != GS4_PROBABIL_PNT) && (templat != GS4_PERCENT_PNT)
       && (templat != GS4_ERROR)
       && (templat != GS4_STATISTIC)
       && (templat != GS4_PROBABIL_TIME) && (templat != GS4_PERCENT_TIME)
       && (templat != GS4_ENSEMBLE_STAT)
       && (templat != GS4_STATISTIC_SPATIAL_AREA)
       && (templat != GS4_RADAR) && (templat != GS4_SATELLITE)
       && (templat != GS4_SATELLITE_SYNTHETIC)
       && (templat != GS4_DERIVED_INTERVAL)
       && (templat != GS4_ANALYSIS_CHEMICAL)
       && (templat != GS4_OPTICAL_PROPERTIES_AEROSOL) ) {
      errSprintf ("This was only designed for templates 0, 1, 2, 5, 6, 7, 8, 9, "
                  "10, 11, 12, 15, 20, 30, 32, 40, 48. Template found = %d\n", templat);

      inv->validTime = 0;
      inv->foreSec = 0;
      reallocSprintf (&(inv->element), "unknown");
      reallocSprintf (&(inv->comment), "unknown");
      reallocSprintf (&(inv->unitName), "unknown");
      reallocSprintf (&(inv->shortFstLevel), "unknown");
      reallocSprintf (&(inv->longFstLevel), "unknown");

        /* Jump past section 5. */
        sectNum = 5;
        if (GRIB2SectJump (fp, gribLen, &sectNum, &secLen) != 0) {
            errSprintf ("ERROR: Problems Jumping past section 5\n");
            return -9;
        }
        /* Jump past section 6. */
        sectNum = 6;
        if (GRIB2SectJump (fp, gribLen, &sectNum, &secLen) != 0) {
            errSprintf ("ERROR: Problems Jumping past section 6\n");
            return -10;
        }
        /* Jump past section 7. */
        sectNum = 7;
        if (GRIB2SectJump (fp, gribLen, &sectNum, &secLen) != 0) {
            errSprintf ("ERROR: Problems Jumping past section 7\n");
            return -11;
        }
        return 1;
   }
   if( secLen < 19 - 5 + 4 )
       return -8;

   cat = (*buffer)[10 - 5];
   subcat = (*buffer)[11 - 5];
   if( templat == GS4_ANALYSIS_CHEMICAL )
   {
        if( secLen < 21 - 5 + 4 )
            return -8;
        genProcess = (*buffer)[14 - 5];
   }
   else if( templat == GS4_OPTICAL_PROPERTIES_AEROSOL )
   {
        if( secLen < 36 - 5 + 1 )
            return -8;
        genProcess = (*buffer)[36 - 5];
   }
   else
        genProcess = (*buffer)[12 - 5];
   genID = 0;
   probType = 0;
   lowerProb = 0;
   upperProb = 0;
   if ((templat == GS4_RADAR) || (templat == GS4_SATELLITE) ||
       (templat == 254)) {
      inv->foreSec = 0;
      inv->validTime = inv->refTime;
      timeIncrType = 255;
      timeRangeUnit = 255;
      lenTime = 0;
   } else {
      int nOffset = 0;
      if( templat == GS4_ANALYSIS_CHEMICAL ) {
          nOffset = 16 - 14;
      }
      else if( templat == GS4_OPTICAL_PROPERTIES_AEROSOL )
      {
          nOffset = 38 - 14;
      }
      genID = (*buffer)[nOffset + 14 - 5];
      /* Compute forecast time. */
      foreTimeUnit = (*buffer)[nOffset + 18 - 5];
      MEMCPY_BIG (&foreTime, *buffer + nOffset + 19 - 5, sizeof (sInt4));
      if (ParseSect4Time2sec (inv->refTime, foreTime, foreTimeUnit, &(inv->foreSec)) != 0) {
         errSprintf ("unable to convert TimeUnit: %d \n", foreTimeUnit);
         return -8;
      }
      /* Compute valid time. */
      inv->validTime = inv->refTime + inv->foreSec;
      timeIncrType = 255;
      timeRangeUnit = 1;
      lenTime = static_cast<sInt4>(
          std::max(static_cast<double>(std::numeric_limits<sInt4>::min()),
          std::min(static_cast<double>(std::numeric_limits<sInt4>::max()),
                   inv->foreSec / 3600.0)));
      switch (templat) {
         case GS4_PROBABIL_PNT: /* 4.5 */
            if( secLen < 44 - 5 + 4)
                return -8;
            probType = (*buffer)[37 - 5];
            factor = sbit_2Comp_oneByte((sChar) (*buffer)[38 - 5]);
            MEMCPY_BIG (&value, *buffer + 39 - 5, sizeof (sInt4));
            value = sbit_2Comp_fourByte(value);
            lowerProb = value * pow (10.0, -1 * factor);
            factor = sbit_2Comp_oneByte((sChar) (*buffer)[43 - 5]);
            MEMCPY_BIG (&value, *buffer + 44 - 5, sizeof (sInt4));
            value = sbit_2Comp_fourByte(value);
            upperProb = value * pow (10.0, -1 * factor);
            break;

         case GS4_PERCENT_PNT: /* 4.6 */
            if( secLen < 35 - 5 + 1)
                return -8;
            percentile = (*buffer)[35 - 5];
            break;

         case GS4_DERIVED_INTERVAL: /* 4.12 */
            if( secLen < 52 - 5 + 4)
                return -8;
            if (InventoryParseTime (*buffer + 37 - 5, &(inv->validTime)) != 0) {
               printf ("Warning: Investigate Template 4.12 bytes 37-43\n");
               inv->validTime = inv->refTime + inv->foreSec;
            }
            timeIncrType = (*buffer)[50 - 5];
            timeRangeUnit = (*buffer)[51 - 5];
            MEMCPY_BIG (&lenTime, *buffer + 52 - 5, sizeof (sInt4));
/* If lenTime == missing (2^32 -1) we might do something, but not with 255.*/
/*
            if (lenTime == 255) {
               lenTime = (inv->validTime -
                          (inv->refTime + inv->foreSec)) / 3600;
            }
*/
            break;

         case GS4_PERCENT_TIME: /* 4.10 */
            if( secLen < 51 - 5 + 4)
                return -8;
            percentile = (*buffer)[35 - 5];
            if (InventoryParseTime (*buffer + 36 - 5, &(inv->validTime)) != 0) {
               printf ("Warning: Investigate Template 4.10 bytes 36-42\n");
               inv->validTime = inv->refTime + inv->foreSec;
            }
            timeIncrType = (*buffer)[49 - 5];
            timeRangeUnit = (*buffer)[50 - 5];
            MEMCPY_BIG (&lenTime, *buffer + 51 - 5, sizeof (sInt4));
/* If lenTime == missing (2^32 -1) we might do something, but not with 255.*/
/*
            if (lenTime == 255) {
               lenTime = (inv->validTime -
                          (inv->refTime + inv->foreSec)) / 3600;
            }
*/
            break;
         case GS4_STATISTIC: /* 4.8 */
            if( secLen < 50 - 5 + 4)
                return -8;
            if (InventoryParseTime (*buffer + 35 - 5, &(inv->validTime)) != 0) {
               printf ("Warning: Investigate Template 4.8 bytes 35-41\n");
               inv->validTime = inv->refTime + inv->foreSec;
            }
            statProcessID = (*buffer)[47 -5];
            timeIncrType = (*buffer)[48 - 5];
            timeRangeUnit = (*buffer)[49 - 5];
            MEMCPY_BIG (&lenTime, *buffer + 50 - 5, sizeof (sInt4));
/* If lenTime == missing (2^32 -1) we might do something, but not with 255.*/
/*
            if (lenTime == 255) {
               lenTime = (inv->validTime -
                          (inv->refTime + inv->foreSec)) / 3600;
            }
*/
            break;
         case GS4_ENSEMBLE_STAT: /* 4.11 */
            if( secLen < 53 - 5 + 4)
                return -8;
            if (InventoryParseTime (*buffer + 38 - 5, &(inv->validTime)) != 0) {
               printf ("Warning: Investigate Template 4.11 bytes 38-44\n");
               inv->validTime = inv->refTime + inv->foreSec;
            }
            timeIncrType = (*buffer)[51 - 5];
            timeRangeUnit = (*buffer)[52 - 5];
            MEMCPY_BIG (&lenTime, *buffer + 53 - 5, sizeof (sInt4));
/* If lenTime == missing (2^32 -1) we might do something, but not with 255.*/
/*
            if (lenTime == 255) {
               lenTime = (inv->validTime -
                          (inv->refTime + inv->foreSec)) / 3600;
            }
*/
            break;
         case GS4_PROBABIL_TIME: /* 4.9 */
            if( secLen < 63 - 5 + 4)
                return -8;
            probType = (*buffer)[37 - 5];
            factor = sbit_2Comp_oneByte((sChar) (*buffer)[38 - 5]);
            MEMCPY_BIG (&value, *buffer + 39 - 5, sizeof (sInt4));
            value = sbit_2Comp_fourByte(value);
            lowerProb = value * pow (10.0, -1 * factor);
            factor = sbit_2Comp_oneByte((sChar) (*buffer)[43 - 5]);
            MEMCPY_BIG (&value, *buffer + 44 - 5, sizeof (sInt4));
            value = sbit_2Comp_fourByte(value);
            upperProb = value * pow (10.0, -1 * factor);

            if (InventoryParseTime (*buffer + 48 - 5, &(inv->validTime)) != 0) {
               printf ("Warning: Investigate Template 4.9 bytes 48-54\n");
               inv->validTime = inv->refTime + inv->foreSec;
            }
            timeIncrType = (*buffer)[61 - 5];
            timeRangeUnit = (*buffer)[62 - 5];
            MEMCPY_BIG (&lenTime, *buffer + 63 - 5, sizeof (sInt4));
/* If lenTime == missing (2^32 -1) we might do something, but not with 255.*/
/*
            if (lenTime == 255) {
               lenTime = (inv->validTime -
                          (inv->refTime + inv->foreSec)) / 3600;
            }
*/
            break;
      }
   }

   uChar derivedFcst = (uChar)-1; // not determined
   switch (templat) {
       case GS4_DERIVED:
       case GS4_DERIVED_CLUSTER_RECTANGULAR_AREA:
       case GS4_DERIVED_CLUSTER_CIRCULAR_AREA:
       case GS4_DERIVED_INTERVAL:
       case GS4_DERIVED_INTERVAL_CLUSTER_RECTANGULAR_AREA:
       case GS4_DERIVED_INTERVAL_CLUSTER_CIRCULAR_AREA:
           if( secLen >= 35 ) {
               derivedFcst = (uChar) (*buffer)[35 - 5];
           }
           break;
       default:
           break;
   }

   if (timeRangeUnit == 255) {
      timeRangeUnit = 1;
      lenTime = DoubleToSInt4Clamp(
          (inv->validTime - inv->foreSec - inv->refTime) / 3600.0);
   }
/*   myAssert (timeRangeUnit == 1);*/
   /* Try to convert lenTime to hourly. */
   if (timeRangeUnit == 0) {
      lenTime = (sInt4) (lenTime / 60.);
      timeRangeUnit = 1;
   } else if (timeRangeUnit == 1) {
   } else if (timeRangeUnit == 2) {
      if( lenTime < INT_MIN / 24 || lenTime > INT_MAX / 24 )
          return -8;
      lenTime = lenTime * 24;
      timeRangeUnit = 1;
   } else if (timeRangeUnit == 10) {
      if( lenTime < INT_MIN / 3 || lenTime > INT_MAX / 3 )
          return -8;
      lenTime = lenTime * 3;
      timeRangeUnit = 1;
   } else if (timeRangeUnit == 11) {
      if( lenTime < INT_MIN / 6 || lenTime > INT_MAX / 6 )
          return -8;
      lenTime = lenTime * 6;
      timeRangeUnit = 1;
   } else if (timeRangeUnit == 12) {
      if( lenTime < INT_MIN / 12 || lenTime > INT_MAX / 12 )
          return -8;
      lenTime = lenTime * 12;
      timeRangeUnit = 1;
   } else if (timeRangeUnit == 13) {
      lenTime = (sInt4) (lenTime / 3600.);
      timeRangeUnit = 1;
   } else if (timeRangeUnit == 3) {  /* month */
      /* Actually use the timeRangeUnit == 3 */
/*
      lenTime = (inv->validTime - Clock_AddMonthYear (inv->validTime, -1 * lenTime, 0)) / 3600.;
      timeRangeUnit = 1;
*/
   } else if (timeRangeUnit == 4) {  /* month */
      /* Actually use the timeRangeUnit == 4 */
/*
      lenTime = (inv->validTime - Clock_AddMonthYear (inv->validTime, 0, -1 * lenTime)) / 3600.;
      timeRangeUnit = 1;
*/
   } else if (timeRangeUnit == 5) {  /* decade */
      if( lenTime < INT_MIN / 10 || lenTime > INT_MAX / 10 )
          return -8;
      lenTime = lenTime * 10;
      timeRangeUnit = 4;
/*
      lenTime = (inv->validTime - Clock_AddMonthYear (inv->validTime, 0, -10 * lenTime)) / 3600.;
      timeRangeUnit = 1;
*/
   } else if (timeRangeUnit == 6) {  /* normal */
      if( lenTime < INT_MIN / 30 || lenTime > INT_MAX / 30 )
          return -8;
      lenTime = lenTime * 30;
      timeRangeUnit = 4;
/*
      lenTime = (inv->validTime - Clock_AddMonthYear (inv->validTime, 0, -30 * lenTime)) / 3600.;
      timeRangeUnit = 1;
*/
   } else if (timeRangeUnit == 7) {  /* century */
      if( lenTime < INT_MIN / 100 || lenTime > INT_MAX / 100 )
          return -8;
      lenTime = lenTime * 100;
      timeRangeUnit = 4;
/*
      lenTime = (inv->validTime - Clock_AddMonthYear (inv->validTime, 0, -100 * lenTime)) / 3600.;
      timeRangeUnit = 1;
*/
   } else {
      printf ("Can't handle this timeRangeUnit\n");
      //myAssert (timeRangeUnit == 1);
      return -8;
   }
   if (lenTime == GRIB2MISSING_s4) {
      lenTime = 0;
   }

   if ((templat == GS4_RADAR) || (templat == GS4_SATELLITE)
       || (templat == GS4_SATELLITE_SYNTHETIC)
       || (templat == 254) || (templat == 1000) || (templat == 1001)
       || (templat == 1002)) {
      fstSurfValue = 0;
      f_fstValue = 0;
      fstSurfType = 0;
      sndSurfValue = 0;
      f_sndValue = 0;
   } else {
      unsigned int nOffset = 0;
      if( templat == GS4_ANALYSIS_CHEMICAL ) {
          nOffset = 16 - 14;
      }
      else if( templat == GS4_OPTICAL_PROPERTIES_AEROSOL )
      {
          nOffset = 38 - 14;
      }

      if( secLen < nOffset + 31 - 5 + 4)
            return -8;
      fstSurfType = (*buffer)[nOffset + 23 - 5];
      unsigned char u_scale = ((unsigned char*)(*buffer))[nOffset + 24 - 5];
      scale = (u_scale & 0x80) ? -(u_scale & 0x7f) : u_scale;
      unsigned int u_value;
      MEMCPY_BIG (&u_value, *buffer + nOffset + 25 - 5, sizeof (u_value));
      value = (u_value & 0x80000000U) ? -static_cast<int>(u_value & 0x7fffffff) : static_cast<int>(u_value);
      if ((value == GRIB2MISSING_s4) || (scale == GRIB2MISSING_s1) ||
          (fstSurfType == GRIB2MISSING_u1)) {
         fstSurfValue = 0;
         f_fstValue = 1;
      } else {
         fstSurfValue = value * pow (10.0, (int) (-1 * scale));
         f_fstValue = 1;
      }
      sndSurfType = (*buffer)[nOffset + 29 - 5];
      u_scale = ((unsigned char*)(*buffer))[nOffset + 30 - 5];
      scale = (u_scale & 0x80) ? -(u_scale & 0x7f) : u_scale;
      MEMCPY_BIG (&u_value, *buffer + nOffset + 31 - 5, sizeof (u_value));
      value = (u_value & 0x80000000U) ? -static_cast<int>(u_value & 0x7fffffff) : static_cast<int>(u_value);
      if ((value == GRIB2MISSING_s4) || (scale == GRIB2MISSING_s1) ||
          (sndSurfType == GRIB2MISSING_u1)) {
         sndSurfValue = 0;
         f_sndValue = 0;
      } else {
         sndSurfValue = value * pow (10.0, -1 * scale);
         f_sndValue = 1;
      }
   }