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