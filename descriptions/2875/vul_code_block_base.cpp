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