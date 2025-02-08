int SHPAPI_CALL
SHPRewindObject( CPL_UNUSED SHPHandle hSHP,
                 SHPObject * psObject )
{
    int  iOpRing, bAltered = 0;

/* -------------------------------------------------------------------- */
/*      Do nothing if this is not a polygon object.                     */
/* -------------------------------------------------------------------- */
    if( psObject->nSHPType != SHPT_POLYGON
        && psObject->nSHPType != SHPT_POLYGONZ
        && psObject->nSHPType != SHPT_POLYGONM )
        return 0;

    if( psObject->nVertices == 0 || psObject->nParts == 0 )
        return 0;

/* -------------------------------------------------------------------- */
/*      Process each of the rings.                                      */
/* -------------------------------------------------------------------- */
    for( iOpRing = 0; iOpRing < psObject->nParts; iOpRing++ )
    {
        int      bInner, iVert, nVertCount, nVertStart, iCheckRing;
        double   dfSum, dfTestX, dfTestY;

        nVertStart = psObject->panPartStart[iOpRing];

        if( iOpRing == psObject->nParts-1 )
            nVertCount = psObject->nVertices - psObject->panPartStart[iOpRing];
        else
            nVertCount = psObject->panPartStart[iOpRing+1]
                - psObject->panPartStart[iOpRing];

        if (nVertCount < 2)
            continue;
/* -------------------------------------------------------------------- */
/*      Determine if this ring is an inner ring or an outer ring        */
/*      relative to all the other rings.  For now we assume the         */
/*      first ring is outer and all others are inner, but eventually    */
/*      we need to fix this to handle multiple island polygons and      */
/*      unordered sets of rings.                                        */
/*                                                                      */
/* -------------------------------------------------------------------- */

        /* Use point in the middle of segment to avoid testing
         * common points of rings.
         */
        dfTestX = ( psObject->padfX[psObject->panPartStart[iOpRing]]
                    + psObject->padfX[psObject->panPartStart[iOpRing] + 1] ) / 2;
        dfTestY = ( psObject->padfY[psObject->panPartStart[iOpRing]]
                    + psObject->padfY[psObject->panPartStart[iOpRing] + 1] ) / 2;

        bInner = FALSE;
        for( iCheckRing = 0; iCheckRing < psObject->nParts; iCheckRing++ )
        {
            int nVertStartCheck, nVertCountCheck;
            int iEdge;

            if( iCheckRing == iOpRing )
                continue;

            nVertStartCheck = psObject->panPartStart[iCheckRing];

            if( iCheckRing == psObject->nParts-1 )
                nVertCountCheck = psObject->nVertices
                    - psObject->panPartStart[iCheckRing];
            else
                nVertCountCheck = psObject->panPartStart[iCheckRing+1]
                    - psObject->panPartStart[iCheckRing];

            for( iEdge = 0; iEdge < nVertCountCheck; iEdge++ )
            {
                int iNext;

                if( iEdge < nVertCountCheck-1 )
                    iNext = iEdge+1;
                else
                    iNext = 0;

                /* Rule #1:
                 * Test whether the edge 'straddles' the horizontal ray from the test point (dfTestY,dfTestY)
                 * The rule #1 also excludes edges colinear with the ray.
                 */
                if ( ( psObject->padfY[iEdge+nVertStartCheck] < dfTestY
                       && dfTestY <= psObject->padfY[iNext+nVertStartCheck] )
                     || ( psObject->padfY[iNext+nVertStartCheck] < dfTestY
                          && dfTestY <= psObject->padfY[iEdge+nVertStartCheck] ) )
                {
                    /* Rule #2:
                     * Test if edge-ray intersection is on the right from the test point (dfTestY,dfTestY)
                     */
                    double const intersect =
                        ( psObject->padfX[iEdge+nVertStartCheck]
                          + ( dfTestY - psObject->padfY[iEdge+nVertStartCheck] )
                          / ( psObject->padfY[iNext+nVertStartCheck] - psObject->padfY[iEdge+nVertStartCheck] )
                          * ( psObject->padfX[iNext+nVertStartCheck] - psObject->padfX[iEdge+nVertStartCheck] ) );

                    if (intersect  < dfTestX)
                    {
                        bInner = !bInner;
                    }
                }
            }
        } /* for iCheckRing */

/* -------------------------------------------------------------------- */
/*      Determine the current order of this ring so we will know if     */
/*      it has to be reversed.                                          */
/* -------------------------------------------------------------------- */

        dfSum = psObject->padfX[nVertStart] * (psObject->padfY[nVertStart+1] - psObject->padfY[nVertStart+nVertCount-1]);
        for( iVert = nVertStart + 1; iVert < nVertStart+nVertCount-1; iVert++ )
        {
            dfSum += psObject->padfX[iVert] * (psObject->padfY[iVert+1] - psObject->padfY[iVert-1]);
        }

        dfSum += psObject->padfX[iVert] * (psObject->padfY[nVertStart] - psObject->padfY[iVert-1]);

/* -------------------------------------------------------------------- */
/*      Reverse if necessary.                                           */
/* -------------------------------------------------------------------- */
        if( (dfSum < 0.0 && bInner) || (dfSum > 0.0 && !bInner) )
        {
            int   i;

            bAltered++;
            for( i = 0; i < nVertCount/2; i++ )
            {
                double dfSaved;

                /* Swap X */
                dfSaved = psObject->padfX[nVertStart+i];
                psObject->padfX[nVertStart+i] =
                    psObject->padfX[nVertStart+nVertCount-i-1];
                psObject->padfX[nVertStart+nVertCount-i-1] = dfSaved;

                /* Swap Y */
                dfSaved = psObject->padfY[nVertStart+i];
                psObject->padfY[nVertStart+i] =
                    psObject->padfY[nVertStart+nVertCount-i-1];
                psObject->padfY[nVertStart+nVertCount-i-1] = dfSaved;

                /* Swap Z */
                if( psObject->padfZ )
                {
                    dfSaved = psObject->padfZ[nVertStart+i];
                    psObject->padfZ[nVertStart+i] =
                        psObject->padfZ[nVertStart+nVertCount-i-1];
                    psObject->padfZ[nVertStart+nVertCount-i-1] = dfSaved;
                }

                /* Swap M */
                if( psObject->padfM )
                {
                    dfSaved = psObject->padfM[nVertStart+i];
                    psObject->padfM[nVertStart+i] =
                        psObject->padfM[nVertStart+nVertCount-i-1];
                    psObject->padfM[nVertStart+nVertCount-i-1] = dfSaved;
                }
            }
        }
    }

    return bAltered;
}