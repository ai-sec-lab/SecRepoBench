static int GRIB2Inventory2to7 (sChar sectNum, DataSource &fp, sInt4 gribLen,
                               uInt4 *buffLen, char **buffer,
                               inventoryType *inv, uChar prodType,
                               unsigned short int center,
                               unsigned short int subcenter)
{
   uInt4 secLen;        /* The length of the current section. */
   sInt4 foreTime;      /* forecast time (NDFD treats as "projection") */
   uChar foreTimeUnit;  /* The time unit of the "forecast time". */
   /* char *element; *//* Holds the name of the current variable. */
   /* char *comment; *//* Holds more comments about current variable. */
   /* char *unitName; *//* Holds the name of the unit [K] [%] .. etc */
   int convert;         /* Enum type of unit conversions (metaname.c),
                         * Conversion method for this variable's unit. */
   uChar cat;           /* General category of Meteo Product. */
   unsigned short int templat; /* The section 4 template number. */
   uChar subcat;        /* Specific subcategory of Product. */
   uChar genProcess;    /* What type of generate process (Analysis,
                           Forecast, Probability Forecast, etc). */
   uChar fstSurfType;   /* Type of the first fixed surface. */
   double fstSurfValue; /* Value of first fixed surface. */
   sInt4 value;         /* The scaled value from GRIB2 file. */
   sChar factor;        /* The scaled factor from GRIB2 file */
   sChar scale;         /* Surface scale as opposed to probability factor. */
   uChar sndSurfType;   /* Type of the second fixed surface. */
   double sndSurfValue; /* Value of second fixed surface. */
   sChar f_sndValue;    /* flag if SndValue is valid. */
   sChar f_fstValue;    /* flag if FstValue is valid. */
   uChar timeRangeUnit;
   sInt4 lenTime;       /* Used by parseTime to tell difference between 8hr
                         * average and 1hr average ozone. */
   uChar genID;         /* The Generating process ID (used for GFS MOS) */
   uChar probType;      /* The probability type */
   double lowerProb;    /* The lower limit on probability forecast if
                         * template 4.5 or 4.9 */
   double upperProb;    /* The upper limit on probability forecast if
                         * template 4.5 or 4.9 */
   uChar timeIncrType;
   sChar percentile = 0;

   if ((sectNum == 2) || (sectNum == 3)) {
      /* Jump past section (2 or 3). */
      sectNum = -1;
      if (GRIB2SectJump (fp, gribLen, &sectNum, &secLen) != 0) {
         errSprintf ("ERROR: Problems Jumping past section 2 || 3\n");
         return -6;
      }
      if ((sectNum != 2) && (sectNum != 3)) {
         errSprintf ("ERROR: Section 2 or 3 mislabeled\n");
         return -5;
      } else if (sectNum == 2) {
         /* Jump past section 3. */
         sectNum = 3;
         if (GRIB2SectJump (fp, gribLen, &sectNum, &secLen) != 0) {
            errSprintf ("ERROR: Problems Jumping past section 3\n");
            return -6;
         }
      }
   }
   /* Read section 4 into buffer. */
   sectNum = 4;
   if (GRIB2SectToBuffer (fp, gribLen, &sectNum, &secLen, buffLen,
                          buffer) != 0) {
      errSprintf ("ERROR: Problems with section 4\n");
      return -7;
   }
/*
enum { GS4_ANALYSIS, GS4_ENSEMBLE, GS4_DERIVED, GS4_PROBABIL_PNT = 5,
   GS4_STATISTIC = 8, GS4_PROBABIL_TIME = 9, GS4_PERCENTILE = 10,
   GS4_RADAR = 20, GS4_SATELLITE = 30
};
*/
   /* Parse the interesting data out of sect 4. */
   MEMCPY_BIG (&templat, *buffer + 8 - 5, sizeof (short int));
   if ((templat != GS4_ANALYSIS) && (templat != GS4_ENSEMBLE)
       && (templat != GS4_DERIVED)
       && (templat != GS4_PROBABIL_PNT) && (templat != GS4_STATISTIC)
       && (templat != GS4_PROBABIL_TIME) && (templat != GS4_PERCENTILE)
       && (templat != GS4_ENSEMBLE_STAT)
       && (templat != GS4_STATISTIC_SPATIAL_AREA)
       && (templat != GS4_RADAR) && (templat != GS4_SATELLITE)
       && (templat != GS4_DERIVED_INTERVAL)) {
      errSprintf ("This was only designed for templates 0, 1, 2, 5, 8, 9, "
                  "10, 11, 12, 15, 20, 30. Template found = %d\n", templat);
      return -8;
   }
   cat = (*buffer)[10 - 5];
   subcat = (*buffer)[11 - 5];
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
      genID = (*buffer)[14 - 5];
      /* Compute forecast time. */
      foreTimeUnit = (*buffer)[18 - 5];
      MEMCPY_BIG (&foreTime, *buffer + 19 - 5, sizeof (sInt4));
      if (ParseSect4Time2sec (/*inv->refTime, */foreTime, foreTimeUnit, &(inv->foreSec)) != 0) {
         errSprintf ("unable to convert TimeUnit: %d \n", foreTimeUnit);
         return -8;
      }
      /* Compute valid time. */
      inv->validTime = inv->refTime + inv->foreSec;
      timeIncrType = 255;
      timeRangeUnit = 1;
      lenTime = (sInt4) (inv->foreSec / 3600);
      switch (templat) {
         // <MASK>
      }
   }

   if (timeRangeUnit == 255) {
      timeRangeUnit = 1;
      lenTime = (sInt4) ((inv->validTime - inv->foreSec - inv->refTime) /
                         3600);
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
   } else {
      printf ("Can't handle this timeRangeUnit\n");
      //myAssert (timeRangeUnit == 1);
      return -8;
   }
   if (lenTime == GRIB2MISSING_s4) {
      lenTime = 0;
   }

   if ((templat == GS4_RADAR) || (templat == GS4_SATELLITE)
       || (templat == 254) || (templat == 1000) || (templat == 1001)
       || (templat == 1002)) {
      fstSurfValue = 0;
      f_fstValue = 0;
      fstSurfType = 0;
      sndSurfValue = 0;
      f_sndValue = 0;
   } else {
      fstSurfType = (*buffer)[23 - 5];
      scale = (*buffer)[24 - 5];
      MEMCPY_BIG (&value, *buffer + 25 - 5, sizeof (sInt4));
      if ((value == GRIB2MISSING_s4) || (scale == GRIB2MISSING_s1) ||
          (fstSurfType == GRIB2MISSING_u1)) {
         fstSurfValue = 0;
         f_fstValue = 1;
      } else {
         fstSurfValue = value * pow (10.0, (int) (-1 * scale));
         f_fstValue = 1;
      }
      sndSurfType = (*buffer)[29 - 5];
      scale = (*buffer)[30 - 5];
      MEMCPY_BIG (&value, *buffer + 31 - 5, sizeof (sInt4));
      if ((value == GRIB2MISSING_s4) || (scale == GRIB2MISSING_s1) ||
          (sndSurfType == GRIB2MISSING_u1)) {
         sndSurfValue = 0;
         f_sndValue = 0;
      } else {
         sndSurfValue = value * pow (10.0, -1 * scale);
         f_sndValue = 1;
      }
   }

   /* Find out what the name of this variable is. */
   ParseElemName (center, subcenter, prodType, templat, cat, subcat,
                  lenTime, timeRangeUnit, timeIncrType, genID, probType, lowerProb,
                  upperProb, &(inv->element), &(inv->comment),
                  &(inv->unitName), &convert, percentile, genProcess,
                  f_fstValue, fstSurfValue, f_sndValue, sndSurfValue);

   if (! f_fstValue) {
      reallocSprintf (&(inv->shortFstLevel), "0 undefined");
      reallocSprintf (&(inv->longFstLevel), "0.000[-] undefined ()");
   } else {
      ParseLevelName (center, subcenter, fstSurfType, fstSurfValue,
                      f_sndValue, sndSurfValue, &(inv->shortFstLevel),
                      &(inv->longFstLevel));
   }

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
   return 0;
}