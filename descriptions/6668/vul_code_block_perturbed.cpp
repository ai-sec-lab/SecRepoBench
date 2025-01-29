
            /*---------------------------------------------------------
             * Alloc array of fields defs, will be filled in further calls
             *--------------------------------------------------------*/
            psTableDef->pasFieldDef =
                    (AVCFieldInfo*)CPLCalloc(psTableDef->numFields,
                                             sizeof(AVCFieldInfo));

            /*---------------------------------------------------------
             * psInfo->iCurItem is the index of the last field def we read.
             * psInfo->numItems is the number of field defs to read,
             *                     including deleted ones.
             *--------------------------------------------------------*/
            psInfo->numItems = AVCE00Str2Int(lineString+38, 4);
            psInfo->iCurItem = 0;
            psInfo->nCurObjectId = 0;  /* We'll use it as a field index */