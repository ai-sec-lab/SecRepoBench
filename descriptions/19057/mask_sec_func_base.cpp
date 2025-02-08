static int GRIB2Inventory2to7 (sChar sectNum, VSILFILE *fp, sInt4 gribLen,
                               uInt4 *buffLen, char **buffer,
                               inventoryType *inv, uChar prodType,
                               unsigned short int center,
                               unsigned short int subcenter,
                               uChar mstrVersion)
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
   uChar statProcessID = 255;
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
   GS4_PERCENT_PNT, GS4_STATISTIC = 8, GS4_PROBABIL_TIME = 9,
   GS4_PERCENT_TIME = 10, GS4_RADAR = 20, GS4_SATELLITE = 30
};
*/
   if( secLen < 11 )
       return -8;

   /* Parse the interesting data out of sect 4. */
   MEMCPY_BIG (&templat, *buffer + 8 - 5, sizeof (short int));
   // <MASK>

   /* Find out what the name of this variable is. */
   ParseElemName (mstrVersion, center, subcenter, prodType, templat, cat, subcat,
                  lenTime, timeRangeUnit, statProcessID, timeIncrType, genID, probType, lowerProb,
                  upperProb,
                  derivedFcst,
                  &(inv->element), &(inv->comment),
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