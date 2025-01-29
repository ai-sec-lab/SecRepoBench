if (parseInfo->iCurItem < (8 + ABS(psTxt->numVerticesLine) +
                                   ABS(psTxt->numVerticesArrow)) && nLen >= 28)
    {
        /*-------------------------------------------------------------
         * One line for each pair of X,Y coordinates
         * (Lines 8 to 8+numVertices-1)
         *------------------------------------------------------------*/
        psTxt->pasVertices[ parseInfo->iCurItem-8 ].x = CPLAtof(pszLine);
        if (parseInfo->nPrecision == AVC_SINGLE_PREC)
            psTxt->pasVertices[ parseInfo->iCurItem-8 ].y = CPLAtof(pszLine+14);
        else
            psTxt->pasVertices[ parseInfo->iCurItem-8 ].y = CPLAtof(pszLine+21);

        parseInfo->iCurItem++;
    }
    else if (parseInfo->iCurItem < parseInfo->numItems &&
             (psTxt->numChars-1)/80 + 1 - (parseInfo->numItems - parseInfo->iCurItem) >= 0 )
    {
        /*-------------------------------------------------------------
         * Last line, contains the text string
         * Note that text can be split in 80 chars chunk and that buffer
         * has been previously initialized with spaces and '\0'-terminated
         *------------------------------------------------------------*/
        int numLines, iLine;
        numLines = (psTxt->numChars-1)/80 + 1;
        iLine = numLines - (parseInfo->numItems - parseInfo->iCurItem);

        if (iLine == numLines-1)
        {
            strncpy((char*)psTxt->pszText+(iLine*80), pszLine,
                    MIN( (int)nLen, (psTxt->numChars - (iLine*80)) ) );
        }
        else
        {
            strncpy((char*)psTxt->pszText+(iLine*80), pszLine, MIN(nLen, 80));
        }

        parseInfo->iCurItem++;
    }
    else
    {
        CPLError(CE_Failure, CPLE_AppDefined,
                 "Error parsing E00 TX6/TX7 line: \"%s\"", pszLine);
        parseInfo->numItems = parseInfo->iCurItem = 0;
        return nullptr;
    }