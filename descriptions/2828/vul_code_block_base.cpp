double adfXYZ[3] = { 0.0, 0.0, 0.0 };

            for( int iEntry = 0;
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
                        GInt32  nValue;
                        memcpy( &nValue, pabyBString, 4 );
                        adfXYZ[iEntry]
                            = static_cast<int>( CPL_MSBWORD32( nValue ) );
                    }
                    else if( EQUAL(pszCoordinateFormat,"BI16") )
                    {
                        GInt16  nValue;
                        memcpy( &nValue, pabyBString, 2 );
                        adfXYZ[iEntry]
                            = static_cast<int>( CPL_MSBWORD16( nValue ) );
                    }
                    else if( EQUAL(pszCoordinateFormat,"BU32") )
                    {
                        GUInt32 nValue;
                        memcpy( &nValue, pabyBString, 4 );
                        adfXYZ[iEntry]
                            = static_cast<GUInt32>( CPL_MSBWORD32( nValue ) );
                    }
                    else if( EQUAL(pszCoordinateFormat,"BU16") )
                    {
                        GUInt16 nValue;
                        memcpy( &nValue, pabyBString, 2 );
                        adfXYZ[iEntry]
                            = static_cast<GUInt16>( CPL_MSBWORD16( nValue ) );
                    }
                    else if( EQUAL(pszCoordinateFormat,"BFP32") )
                    {
                        float   fValue;

                        memcpy( &fValue, pabyBString, 4 );
                        CPL_MSBPTR32( &fValue );
                        adfXYZ[iEntry] = fValue;
                    }
                    else if( EQUAL(pszCoordinateFormat,"BFP64") )
                    {
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

            padfX[iVertex] = dfXOffset + adfXYZ[0] * dfXScale;
            padfY[iVertex] = dfYOffset + adfXYZ[1] * dfYScale;
            padfZ[iVertex] = adfXYZ[2];