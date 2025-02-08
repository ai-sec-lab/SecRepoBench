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
        int      bInner, iVert, nVertexCount, nVertStart, iCheckRing;
        double   dfSum, dfTestX, dfTestY;

        // <MASK>
        for( iVert = nVertStart + 1; iVert < nVertStart+nVertexCount-1; iVert++ )
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
            for( i = 0; i < nVertexCount/2; i++ )
            {
                double dfSaved;

                /* Swap X */
                dfSaved = psObject->padfX[nVertStart+i];
                psObject->padfX[nVertStart+i] =
                    psObject->padfX[nVertStart+nVertexCount-i-1];
                psObject->padfX[nVertStart+nVertexCount-i-1] = dfSaved;

                /* Swap Y */
                dfSaved = psObject->padfY[nVertStart+i];
                psObject->padfY[nVertStart+i] =
                    psObject->padfY[nVertStart+nVertexCount-i-1];
                psObject->padfY[nVertStart+nVertexCount-i-1] = dfSaved;

                /* Swap Z */
                if( psObject->padfZ )
                {
                    dfSaved = psObject->padfZ[nVertStart+i];
                    psObject->padfZ[nVertStart+i] =
                        psObject->padfZ[nVertStart+nVertexCount-i-1];
                    psObject->padfZ[nVertStart+nVertexCount-i-1] = dfSaved;
                }

                /* Swap M */
                if( psObject->padfM )
                {
                    dfSaved = psObject->padfM[nVertStart+i];
                    psObject->padfM[nVertStart+i] =
                        psObject->padfM[nVertStart+nVertexCount-i-1];
                    psObject->padfM[nVertStart+nVertexCount-i-1] = dfSaved;
                }
            }
        }
    }

    return bAltered;
}