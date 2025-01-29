--------------------------------------------------------------- */
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
            int iEdge;

            if( iCheckRing == iOpRing )
                continue;

            nVertStart = psObject->panPartStart[iCheckRing];

            if( iCheckRing == psObject->nParts-1 )
                nVertexCount = psObject->nVertices
                    - psObject->panPartStart[iCheckRing];
            else
                nVertexCount = psObject->panPartStart[iCheckRing+1]
                    - psObject->panPartStart[iCheckRing];

            for( iEdge = 0; iEdge < nVertexCount; iEdge++ )
            {
                int iNext;

                if( iEdge < nVertexCount-1 )
                    iNext = iEdge+1;
                else
                    iNext = 0;

                /* Rule #1:
                 * Test whether the edge 'straddles' the horizontal ray from the test point (dfTestY,dfTestY)
                 * The rule #1 also excludes edges colinear with the ray.
                 */
                if ( ( psObject->padfY[iEdge+nVertStart] < dfTestY
                       && dfTestY <= psObject->padfY[iNext+nVertStart] )
                     || ( psObject->padfY[iNext+nVertStart] < dfTestY
                          && dfTestY <= psObject->padfY[iEdge+nVertStart] ) )
                {
                    /* Rule #2:
                     * Test if edge-ray intersection is on the right from the test point (dfTestY,dfTestY)
                     */
                    double const intersect =
                        ( psObject->padfX[iEdge+nVertStart]
                          + ( dfTestY - psObject->padfY[iEdge+nVertStart] )
                          / ( psObject->padfY[iNext+nVertStart] - psObject->padfY[iEdge+nVertStart] )
                          * ( psObject->padfX[iNext+nVertStart] - psObject->padfX[iEdge+nVertStart] ) );

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
        nVertStart = psObject->panPartStart[iOpRing];

        if( iOpRing == psObject->nParts-1 )
            nVertexCount = psObject->nVertices - psObject->panPartStart[iOpRing];
        else
            nVertexCount = psObject->panPartStart[iOpRing+1]
                - psObject->panPartStart[iOpRing];

        if (nVertexCount < 2)
            continue;

        dfSum = psObject->padfX[nVertStart] * (psObject->padfY[nVertStart+1] - psObject->padfY[nVertStart+nVertexCount-1]);