AVCTxt   *AVCE00ParseNextTx6Line(AVCE00ParseInfo *psInfo, const char *pszLine)
{
    AVCTxt *psTxt;
    int     i;
    size_t  nLen;

    CPLAssert(psInfo->eFileType == AVCFileTX6);

    psTxt = psInfo->cur.psTxt;

    nLen = strlen(pszLine);

    if (psInfo->numItems == 0)
    {
        /*-------------------------------------------------------------
         * Begin processing a new object, read header line:
         *------------------------------------------------------------*/
        if (nLen < 70)
        {
            CPLError(CE_Failure, CPLE_AppDefined,
                     "Error parsing E00 TX6/TX7 line: \"%s\"", pszLine);
            return nullptr;
        }
        else
        {
            int numVertices;
            /*---------------------------------------------------------
             * System Id is not stored in the E00 file.  Annotations are
             * stored in increasing order of System Id, starting at 1...
             * so we just increment the previous value.
             *--------------------------------------------------------*/
            psTxt->nTxtId = ++psInfo->nCurObjectId;

            psTxt->nUserId         = AVCE00Str2Int(pszLine, 10);
            psTxt->nLevel          = AVCE00Str2Int(pszLine+10, 10);
            psTxt->numVerticesLine = AVCE00Str2Int(pszLine+20, 10);
            psTxt->numVerticesArrow= AVCE00Str2Int(pszLine+30, 10);
            psTxt->nSymbol         = AVCE00Str2Int(pszLine+40, 10);
            psTxt->n28             = AVCE00Str2Int(pszLine+50, 10);
            psTxt->numChars        = AVCE00Str2Int(pszLine+60, 10);

            /*---------------------------------------------------------
             * Realloc the string buffer and array of vertices
             *--------------------------------------------------------*/
            psTxt->pszText = (GByte *)CPLRealloc(psTxt->pszText,
                                                 (psTxt->numChars+1)*
                                                 sizeof(GByte));

            numVertices = ABS(psTxt->numVerticesLine) +
                                 ABS(psTxt->numVerticesArrow);
            if (numVertices > 0)
                psTxt->pasVertices = (AVCVertex*)CPLRealloc(psTxt->pasVertices,
                                              numVertices*sizeof(AVCVertex));

            /*---------------------------------------------------------
             * Fill the whole string buffer with spaces we'll just
             * paste lines in it using strncpy()
             *--------------------------------------------------------*/
            memset(psTxt->pszText, ' ', psTxt->numChars);
            psTxt->pszText[psTxt->numChars] = '\0';

            /*---------------------------------------------------------
             * psInfo->iCurItem is the index of the last line that was read.
             * psInfo->numItems is the number of lines to read.
             *--------------------------------------------------------*/
            psInfo->iCurItem = 0;
            psInfo->numItems = 8 + numVertices + ((psTxt->numChars-1)/80 + 1);
        }
    }
    else if (psInfo->iCurItem < psInfo->numItems &&
             psInfo->iCurItem < 6 && nLen >=60)
    {
        /*-------------------------------------------------------------
         * Text Justification stuff... 2 sets of 20 int16 values.
         *------------------------------------------------------------*/
        GInt16  *pValue;
        int     numValPerLine=7;

        if (psInfo->iCurItem < 3)
            pValue = psTxt->anJust2 + psInfo->iCurItem * 7;
        else
            pValue = psTxt->anJust1 + (psInfo->iCurItem-3) * 7;

        /* Last line of each set contains only 6 values instead of 7 */
        if (psInfo->iCurItem == 2 || psInfo->iCurItem == 5)
            numValPerLine = 6;

        for(i=0; i<numValPerLine; i++)
            pValue[i] = (GInt16)AVCE00Str2Int(pszLine + i*10, 10);

        psInfo->iCurItem++;
    }
    else if (psInfo->iCurItem < psInfo->numItems &&
             psInfo->iCurItem == 6 && nLen >=14)
    {
        /*-------------------------------------------------------------
         * Line with a -1.000E+02 value, ALWAYS SINGLE PRECISION !!!
         *------------------------------------------------------------*/
        psTxt->f_1e2 = (float)CPLAtof(pszLine);
        psInfo->iCurItem++;
    }
    else if (psInfo->iCurItem < psInfo->numItems &&
             psInfo->iCurItem == 7 && nLen >=42)
    {
        /*-------------------------------------------------------------
         * Line with 3 values, 1st value is text height.
         *------------------------------------------------------------*/
        psTxt->dHeight = CPLAtof(pszLine);
        if (psInfo->nPrecision == AVC_SINGLE_PREC)
        {
            psTxt->dV2     = CPLAtof(pszLine+14);
            psTxt->dV3     = CPLAtof(pszLine+28);
        }
        else
        {
            psTxt->dV2     = CPLAtof(pszLine+21);
            psTxt->dV3     = CPLAtof(pszLine+42);
        }

        psInfo->iCurItem++;
    }
    else if (psInfo->iCurItem < (8 + ABS(psTxt->numVerticesLine) +
                                   ABS(psTxt->numVerticesArrow)) && nLen >= 28)
    {
        /*-------------------------------------------------------------
         * One line for each pair of X,Y coordinates
         * (Lines 8 to 8+numVertices-1)
         *------------------------------------------------------------*/
        psTxt->pasVertices[ psInfo->iCurItem-8 ].x = CPLAtof(pszLine);
        if (psInfo->nPrecision == AVC_SINGLE_PREC)
            psTxt->pasVertices[ psInfo->iCurItem-8 ].y = CPLAtof(pszLine+14);
        else
            psTxt->pasVertices[ psInfo->iCurItem-8 ].y = CPLAtof(pszLine+21);

        psInfo->iCurItem++;
    }
    else if (psInfo->iCurItem < psInfo->numItems &&
             (psTxt->numChars-1)/80 + 1 - (psInfo->numItems - psInfo->iCurItem) >= 0 )
    {
        /*-------------------------------------------------------------
         * Last line, contains the text string
         * Note that text can be split in 80 chars chunk and that buffer
         * has been previously initialized with spaces and '\0'-terminated
         *------------------------------------------------------------*/
        int numLines, iLine;
        numLines = (psTxt->numChars-1)/80 + 1;
        iLine = numLines - (psInfo->numItems - psInfo->iCurItem);

        if (iLine == numLines-1)
        {
            strncpy((char*)psTxt->pszText+(iLine*80), pszLine,
                    MIN( (int)nLen, (psTxt->numChars - (iLine*80)) ) );
        }
        else
        {
            strncpy((char*)psTxt->pszText+(iLine*80), pszLine, MIN(nLen, 80));
        }

        psInfo->iCurItem++;
    }
    else
    {
        CPLError(CE_Failure, CPLE_AppDefined,
                 "Error parsing E00 TX6/TX7 line: \"%s\"", pszLine);
        psInfo->numItems = psInfo->iCurItem = 0;
        return nullptr;
    }

    /*-----------------------------------------------------------------
     * If we're done parsing this TX6/TX7, then reset the ParseInfo,
     * and return a reference to the TXT structure
     * Otherwise return nullptr, which means that we are expecting more
     * more lines of input.
     *----------------------------------------------------------------*/
    if (psInfo->iCurItem >= psInfo->numItems)
    {
        psInfo->numItems = psInfo->iCurItem = 0;
        return psTxt;
    }

    return nullptr;
}