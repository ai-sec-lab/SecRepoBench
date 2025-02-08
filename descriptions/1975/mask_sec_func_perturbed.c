GXFHandle GXFOpen( const char * filenamePath )

{
    VSILFILE	*fp;
    GXFInfo_t	*psGXF;
    char	szTitle[71];
    char	**papszList;
    int     nHeaderCount = 0;

/* -------------------------------------------------------------------- */
/*      We open in binary to ensure that we can efficiently seek()      */
/*      to any location when reading scanlines randomly.  If we         */
/*      opened as text we might still be able to seek(), but I          */
/*      believe that on Windows, the C library has to read through      */
/*      all the data to find the right spot taking into account DOS     */
/*      CRs.                                                            */
/* -------------------------------------------------------------------- */
    fp = VSIFOpenL( filenamePath, "rb" );

    if( fp == NULL )
    {
        /* how to effectively communicate this error out? */
        CPLError( CE_Failure, CPLE_OpenFailed,
                  "Unable to open file: %s\n", filenamePath );
        return NULL;
    }

/* -------------------------------------------------------------------- */
/*      Create the GXF Information object.                              */
/* -------------------------------------------------------------------- */
    psGXF = (GXFInfo_t *) VSICalloc( sizeof(GXFInfo_t), 1 );
    psGXF->fp = fp;
    psGXF->dfTransformScale = 1.0;
    psGXF->nSense = GXFS_LL_RIGHT;
    psGXF->dfXPixelSize = 1.0;
    psGXF->dfYPixelSize = 1.0;
    psGXF->dfSetDummyTo = -1e12;

    psGXF->dfUnitToMeter = 1.0;
    psGXF->pszTitle = VSIStrdup("");

/* -------------------------------------------------------------------- */
/*      Read the header, one line at a time.                            */
/* -------------------------------------------------------------------- */
    while( (papszList = GXFReadHeaderValue( fp, szTitle)) != NULL && nHeaderCount < MAX_HEADER_COUNT )
    {
        if( STARTS_WITH_CI(szTitle, "#TITL") )
        {
            CPLFree( psGXF->pszTitle );
            psGXF->pszTitle = CPLStrdup( papszList[0] );
        }
        else if( STARTS_WITH_CI(szTitle, "#POIN") )
        {
            psGXF->nRawXSize = atoi(papszList[0]);
        }
        else if( STARTS_WITH_CI(szTitle, "#ROWS") )
        {
            psGXF->nRawYSize = atoi(papszList[0]);
        }
        else if( STARTS_WITH_CI(szTitle, "#PTSE") )
        {
            psGXF->dfXPixelSize = CPLAtof(papszList[0]);
        }
        else if( STARTS_WITH_CI(szTitle, "#RWSE") )
        {
            psGXF->dfYPixelSize = CPLAtof(papszList[0]);
        }
        else if( STARTS_WITH_CI(szTitle, "#DUMM") )
        {
            memset( psGXF->szDummy, 0, sizeof(psGXF->szDummy));
            strncpy( psGXF->szDummy, papszList[0], sizeof(psGXF->szDummy) - 1);
            psGXF->dfSetDummyTo = CPLAtof(papszList[0]);
        }
        else if( STARTS_WITH_CI(szTitle, "#XORI") )
        {
            psGXF->dfXOrigin = CPLAtof(papszList[0]);
        }
        else if( STARTS_WITH_CI(szTitle, "#YORI") )
        {
            psGXF->dfYOrigin = CPLAtof(papszList[0]);
        }
        else if( STARTS_WITH_CI(szTitle, "#ZMIN") )
        {
            psGXF->dfZMinimum = CPLAtof(papszList[0]);
        }
        else if( STARTS_WITH_CI(szTitle, "#ZMAX") )
        {
            psGXF->dfZMaximum = CPLAtof(papszList[0]);
        }
        else if( STARTS_WITH_CI(szTitle, "#SENS") )
        {
            psGXF->nSense = atoi(papszList[0]);
        }
        else if( STARTS_WITH_CI(szTitle,"#MAP_PROJECTION") &&
                 psGXF->papszMapProjection == NULL )
        {
            psGXF->papszMapProjection = papszList;
            papszList = NULL;
        }
        else if( STARTS_WITH_CI(szTitle,"#MAP_D") &&
                 psGXF->papszMapDatumTransform == NULL  )
        {
            psGXF->papszMapDatumTransform = papszList;
            papszList = NULL;
        }
        else if( STARTS_WITH_CI(szTitle, "#UNIT") &&
                 psGXF->pszUnitName == NULL )
        {
            char	**papszFields;

            papszFields = CSLTokenizeStringComplex( papszList[0], ", ",
                                                    TRUE, TRUE );

            if( CSLCount(papszFields) > 1 )
            {
                psGXF->pszUnitName = VSIStrdup( papszFields[0] );
                psGXF->dfUnitToMeter = CPLAtof( papszFields[1] );
                if( psGXF->dfUnitToMeter == 0.0 )
                    psGXF->dfUnitToMeter = 1.0;
            }

            CSLDestroy( papszFields );
        }
        else if( STARTS_WITH_CI(szTitle, "#TRAN") &&
                 psGXF->pszTransformName == NULL )
        {
            char	**papszFields;

            papszFields = CSLTokenizeStringComplex( papszList[0], ", ",
                                                    TRUE, TRUE );

            if( CSLCount(papszFields) > 1 )
            {
                psGXF->dfTransformScale = CPLAtof(papszFields[0]);
                psGXF->dfTransformOffset = CPLAtof(papszFields[1]);
            }

            if( CSLCount(papszFields) > 2 )
                psGXF->pszTransformName = CPLStrdup( papszFields[2] );

            CSLDestroy( papszFields );
        }
        else if( STARTS_WITH_CI(szTitle,"#GTYPE") )
        {
            // <MASK>
        }

        CSLDestroy( papszList );
        nHeaderCount ++;
    }

    CSLDestroy( papszList );

/* -------------------------------------------------------------------- */
/*      Did we find the #GRID?                                          */
/* -------------------------------------------------------------------- */
    if( !STARTS_WITH_CI(szTitle, "#GRID") )
    {
        GXFClose( psGXF );
        CPLError( CE_Failure, CPLE_WrongFormat,
                  "Didn't parse through to #GRID successfully in.\n"
                  "file `%s'.\n",
                  filenamePath );

        return NULL;
    }

/* -------------------------------------------------------------------- */
/*      Allocate, and initialize the raw scanline offset array.         */
/* -------------------------------------------------------------------- */
    if( psGXF->nRawYSize <= 0 )
    {
        GXFClose( psGXF );
        return NULL;
    }

    psGXF->panRawLineOffset = (vsi_l_offset *)
        VSICalloc( sizeof(vsi_l_offset), psGXF->nRawYSize+1 );
    if( psGXF->panRawLineOffset == NULL )
    {
        GXFClose( psGXF );
        return NULL;
    }

    psGXF->panRawLineOffset[0] = VSIFTellL( psGXF->fp );

/* -------------------------------------------------------------------- */
/*      Update the zmin/zmax values to take into account #TRANSFORM     */
/*      information.                                                    */
/* -------------------------------------------------------------------- */
    if( psGXF->dfZMinimum != 0.0 || psGXF->dfZMaximum != 0.0 )
    {
        psGXF->dfZMinimum = (psGXF->dfZMinimum * psGXF->dfTransformScale)
            			+ psGXF->dfTransformOffset;
        psGXF->dfZMaximum = (psGXF->dfZMaximum * psGXF->dfTransformScale)
            			+ psGXF->dfTransformOffset;
    }

    return( (GXFHandle) psGXF );
}