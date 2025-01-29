case GS4_PROBABIL_PNT: /* 4.5 */
            if( *buffLen <= 44 - 5 + 4)
                return -8;
            probType = (*buffer)[37 - 5];
            factor = (sChar) (*buffer)[38 - 5];
            MEMCPY_BIG (&value, *buffer + 39 - 5, sizeof (sInt4));
            lowerProb = value * pow (10.0, -1 * factor);
            factor = (sChar) (*buffer)[43 - 5];
            MEMCPY_BIG (&value, *buffer + 44 - 5, sizeof (sInt4));
            upperProb = value * pow (10.0, -1 * factor);
            break;
         case GS4_DERIVED_INTERVAL: /* 4.12 */
            if( *buffLen <= 52 - 5 + 4)
                return -8;
            if (InventoryParseTime (*buffer + 37 - 5, &(forecastData->validTime)) != 0) {
               printf ("Warning: Investigate Template 4.12 bytes 37-43\n");
               forecastData->validTime = forecastData->refTime + forecastData->foreSec;
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
         case GS4_PERCENTILE: /* 4.10 */
            if( *buffLen <= 51 - 5 + 4)
                return -8;
            percentile = (*buffer)[35 - 5];
            if (InventoryParseTime (*buffer + 36 - 5, &(forecastData->validTime)) != 0) {
               printf ("Warning: Investigate Template 4.10 bytes 36-42\n");
               forecastData->validTime = forecastData->refTime + forecastData->foreSec;
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
            if( *buffLen <= 50 - 5 + 4)
                return -8;
            if (InventoryParseTime (*buffer + 35 - 5, &(forecastData->validTime)) != 0) {
               printf ("Warning: Investigate Template 4.8 bytes 35-41\n");
               forecastData->validTime = forecastData->refTime + forecastData->foreSec;
            }
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
            if( *buffLen <= 53 - 5 + 4)
                return -8;
            if (InventoryParseTime (*buffer + 38 - 5, &(forecastData->validTime)) != 0) {
               printf ("Warning: Investigate Template 4.11 bytes 38-44\n");
               forecastData->validTime = forecastData->refTime + forecastData->foreSec;
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
            if( *buffLen <= 63 - 5 + 4)
                return -8;
            probType = (*buffer)[37 - 5];
            if ((uChar) (*buffer)[38 - 5] > 128) {
               factor = 128 - (uChar) (*buffer)[38 - 5];
            } else {
               factor = (*buffer)[38 - 5];
            }
            MEMCPY_BIG (&value, *buffer + 39 - 5, sizeof (sInt4));
            lowerProb = value * pow (10.0, -1 * factor);

            if ((uChar) (*buffer)[43 - 5] > 128) {
               factor = 128 - (uChar) (*buffer)[43 - 5];
            } else {
               factor = (*buffer)[43 - 5];
            }
            MEMCPY_BIG (&value, *buffer + 44 - 5, sizeof (sInt4));
            upperProb = value * pow (10.0, -1 * factor);

            if (InventoryParseTime (*buffer + 48 - 5, &(forecastData->validTime)) != 0) {
               printf ("Warning: Investigate Template 4.9 bytes 48-54\n");
               forecastData->validTime = forecastData->refTime + forecastData->foreSec;
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