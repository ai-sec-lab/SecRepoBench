int SDTS_IREF::GetSADR( DDFField * pField, int nVertices,
                        double *padfX, double * padfY, double * padfZ )

{
/* -------------------------------------------------------------------- */
/*      For the sake of efficiency we depend on our knowledge that      */
/*      the SADR field is a series of bigendian int32's and decode      */
/*      them directly.                                                  */
/* -------------------------------------------------------------------- */
    if( nDefaultSADRFormat
        && pField->GetFieldDefn()->GetSubfieldCount() == 2 )
    {
        if( pField->GetDataSize() < nVertices * SDTS_SIZEOF_SADR )
        {
            return FALSE;
        }

        GInt32          anXY[2];
        const char      *pachRawData = pField->GetData();

        for( int iVertex = 0; iVertex < nVertices; iVertex++ )
        {
            // we copy to a temp buffer to ensure it is world aligned.
            memcpy( anXY, pachRawData, 8 );
            pachRawData += 8;

            // possibly byte swap, and always apply scale factor
            padfX[iVertex] = dfXOffset
                + dfXScale * static_cast<int>( CPL_MSBWORD32( anXY[0] ) );
            padfY[iVertex] = dfYOffset
                + dfYScale * static_cast<int>( CPL_MSBWORD32( anXY[1] ) );

            padfZ[iVertex] = 0.0;
        }
    }

/* -------------------------------------------------------------------- */
/*      This is the generic case.  We assume either two or three        */
/*      subfields, and treat these as X, Y and Z regardless of          */
/*      name.                                                           */
/* -------------------------------------------------------------------- */
    else
    {
        DDFFieldDefn    *poFieldDefn = pField->GetFieldDefn();
        int             nBytesRemaining = pField->GetDataSize();
        const char     *pachFieldData = pField->GetData();

        if( poFieldDefn->GetSubfieldCount() != 2 &&
            poFieldDefn->GetSubfieldCount() != 3 )
        {
            return FALSE;
        }

        for( int iVertex = 0; iVertex < nVertices; iVertex++ )
        {
            // <MASK>
        } /* next iVertex */
    }

    return TRUE;
}