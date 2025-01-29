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