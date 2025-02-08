int SDTS_IREF::GetSADR( DDFField * poField, int nVertices,
                        double *padfX, double * padfY, double * padfZ )

{
/* -------------------------------------------------------------------- */
/*      For the sake of efficiency we depend on our knowledge that      */
/*      the SADR field is a series of bigendian int32's and decode      */
/*      them directly.                                                  */
/* -------------------------------------------------------------------- */
    if( nDefaultSADRFormat
        && poField->GetFieldDefn()->GetSubfieldCount() == 2 )
    {
        if( poField->GetDataSize() < nVertices * SDTS_SIZEOF_SADR )
        {
            return FALSE;
        }

        GInt32          anXY[2];
        const char      *pachRawData = poField->GetData();

        for( int vertexIndex = 0; vertexIndex < nVertices; vertexIndex++ )
        {
            // we copy to a temp buffer to ensure it is world aligned.
            memcpy( anXY, pachRawData, 8 );
            pachRawData += 8;

            // possibly byte swap, and always apply scale factor
            padfX[vertexIndex] = dfXOffset
                + dfXScale * static_cast<int>( CPL_MSBWORD32( anXY[0] ) );
            padfY[vertexIndex] = dfYOffset
                + dfYScale * static_cast<int>( CPL_MSBWORD32( anXY[1] ) );

            padfZ[vertexIndex] = 0.0;
        }
    }

/* -------------------------------------------------------------------- */
/*      This is the generic case.  We assume either two or three        */
/*      subfields, and treat these as X, Y and Z regardless of          */
/*      name.                                                           */
/* -------------------------------------------------------------------- */
    else
    {
        DDFFieldDefn    *poFieldDefn = poField->GetFieldDefn();
        int             nBytesRemaining = poField->GetDataSize();
        const char     *pachFieldData = poField->GetData();

        if( poFieldDefn->GetSubfieldCount() != 2 &&
            poFieldDefn->GetSubfieldCount() != 3 )
        {
            return FALSE;
        }

        for( int vertexIndex = 0; vertexIndex < nVertices; vertexIndex++ )
        {
            double adfXYZ[3] = { 0.0, 0.0, 0.0 };

            for( int iEntry = 0;
                 nBytesRemaining > 0 &&
                 iEntry < poFieldDefn->GetSubfieldCount();
                 iEntry++ )
            {
                int nBytesConsumed = 0;
                DDFSubfieldDefn *poSF = poFieldDefn->GetSubfield(iEntry);

                switch( poSF->GetType() )
                {
                  case DDFInt:
                    adfXYZ[iEntry] =
                        poSF->ExtractIntData( pachFieldData,
                                              nBytesRemaining,
                                              &nBytesConsumed );
                    break;

                  case DDFFloat:
                    adfXYZ[iEntry] =
                        poSF->ExtractFloatData( pachFieldData,
                                                nBytesRemaining,
                                                &nBytesConsumed );
                    break;

                  case DDFBinaryString:
                    {
                      GByte *pabyBString = reinterpret_cast<GByte *> (
                          const_cast<char *>(
                              poSF->ExtractStringData( pachFieldData,
                                                       nBytesRemaining,
                                                       &nBytesConsumed ) ) );

                    if( EQUAL(pszCoordinateFormat,"BI32") )
                    {
                        if( nBytesConsumed < 4 )
                            return FALSE;
                        GInt32  nValue;
                        memcpy( &nValue, pabyBString, 4 );
                        adfXYZ[iEntry]
                            = static_cast<int>( CPL_MSBWORD32( nValue ) );
                    }
                    else if( EQUAL(pszCoordinateFormat,"BI16") )
                    {
                        if( nBytesConsumed < 2 )
                            return FALSE;
                        GInt16  nValue;
                        memcpy( &nValue, pabyBString, 2 );
                        adfXYZ[iEntry]
                            = static_cast<int>( CPL_MSBWORD16( nValue ) );
                    }
                    else if( EQUAL(pszCoordinateFormat,"BU32") )
                    {
                        if( nBytesConsumed < 4 )
                            return FALSE;
                        GUInt32 nValue;
                        memcpy( &nValue, pabyBString, 4 );
                        adfXYZ[iEntry]
                            = static_cast<GUInt32>( CPL_MSBWORD32( nValue ) );
                    }
                    else if( EQUAL(pszCoordinateFormat,"BU16") )
                    {
                        if( nBytesConsumed < 2 )
                            return FALSE;
                        GUInt16 nValue;
                        memcpy( &nValue, pabyBString, 2 );
                        adfXYZ[iEntry]
                            = static_cast<GUInt16>( CPL_MSBWORD16( nValue ) );
                    }
                    else if( EQUAL(pszCoordinateFormat,"BFP32") )
                    {
                        if( nBytesConsumed < 4 )
                            return FALSE;
                        float   fValue;

                        memcpy( &fValue, pabyBString, 4 );
                        CPL_MSBPTR32( &fValue );
                        adfXYZ[iEntry] = fValue;
                    }
                    else if( EQUAL(pszCoordinateFormat,"BFP64") )
                    {
                        if( nBytesConsumed < 8 )
                            return FALSE;
                        double  dfValue;

                        memcpy( &dfValue, pabyBString, 8 );
                        CPL_MSBPTR64( &dfValue );
                        adfXYZ[iEntry] = dfValue;
                    }
                    }
                    break;

                  default:
                    adfXYZ[iEntry] = 0.0;
                    break;
                }

                pachFieldData += nBytesConsumed;
                nBytesRemaining -= nBytesConsumed;
            } /* next iEntry */

            padfX[vertexIndex] = dfXOffset + adfXYZ[0] * dfXScale;
            padfY[vertexIndex] = dfYOffset + adfXYZ[1] * dfYScale;
            padfZ[vertexIndex] = adfXYZ[2];
        } /* next iVertex */
    }

    return TRUE;
}