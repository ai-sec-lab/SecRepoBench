int NTFFileReader::Open( const char * pszFilenameIn )

{
    if( pszFilenameIn != NULL )
    {
        ClearDefs();

        CPLFree( pszFilename );
        pszFilename = CPLStrdup( pszFilenameIn );
    }
    else
        Close();

/* -------------------------------------------------------------------- */
/*      Open the file.                                                  */
/* -------------------------------------------------------------------- */
    fp = VSIFOpenL( pszFilename, "rb" );

    // notdef: we should likely issue a proper CPL error message based
    // based on errno here.
    if( fp == NULL )
    {
        CPLError( CE_Failure, CPLE_OpenFailed,
                  "Unable to open file `%s' for read access.\n",
                  pszFilename );
        return FALSE;
    }

/* -------------------------------------------------------------------- */
/*      If we are just reopening an existing file we will just scan     */
/*      past the section header ... no need to reform all the definitions.*/
/* -------------------------------------------------------------------- */
    if( pszFilenameIn == NULL )
    {
        NTFRecord *poRecord = NULL;

        for( poRecord = new NTFRecord( fp );
             poRecord->GetType() != NRT_VTR && poRecord->GetType() != NRT_SHR;
             poRecord = new NTFRecord( fp ) )
        {
            delete poRecord;
        }

        delete poRecord;

        return TRUE;
    }

/* -------------------------------------------------------------------- */
/*      Read the first record, and verify it is a proper volume header. */
/* -------------------------------------------------------------------- */
    NTFRecord      oVHR( fp );

    if( oVHR.GetType() != NRT_VHR )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "File `%s' appears to not be a UK NTF file.\n",
                  pszFilename );
        return FALSE;
    }

    nNTFLevel = atoi(oVHR.GetField( 57, 57 ));
    if( !( nNTFLevel >= 1 && nNTFLevel <= 5 ) )
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Invalid value : nNTFLevel = %d", nNTFLevel );
        return FALSE;
    }

/* -------------------------------------------------------------------- */
/*      Read records till we get the section header.                    */
/* -------------------------------------------------------------------- */
    NTFRecord *poRecord = NULL;

    for( poRecord = new NTFRecord( fp );
         poRecord->GetType() != NRT_VTR && poRecord->GetType() != NRT_SHR;
         poRecord = new NTFRecord( fp ) )
    {
// <MASK>

        delete poRecord;
    }

/* -------------------------------------------------------------------- */
/*      Did we fall off the end without finding what we were looking    */
/*      for?                                                            */
/* -------------------------------------------------------------------- */
    if( poRecord->GetType() == NRT_VTR )
    {
        delete poRecord;
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Cound not find section header record in %s.\n",
                  pszFilename );
        return FALSE;
    }

    if( pszProduct == NULL )
    {
        delete poRecord;
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Cound not find product type in %s.\n",
                  pszFilename );
        return FALSE;
    }

/* -------------------------------------------------------------------- */
/*      Classify the product type.                                      */
/* -------------------------------------------------------------------- */
    if( STARTS_WITH_CI(pszProduct, "LAND-LINE") && CPLAtof(pszPVName+5) < 1.3 )
        nProduct = NPC_LANDLINE;
    else if( STARTS_WITH_CI(pszProduct, "LAND-LINE") )
        nProduct = NPC_LANDLINE99;
    else if( EQUAL(pszProduct,"OS_LANDRANGER_CONT") ) // Panorama
        nProduct = NPC_LANDRANGER_CONT;
    else if( EQUAL(pszProduct,"L-F_PROFILE_CON") ) // Panorama
        nProduct = NPC_LANDFORM_PROFILE_CONT;
    else if( STARTS_WITH_CI(pszProduct, "Strategi") )
        nProduct = NPC_STRATEGI;
    else if( STARTS_WITH_CI(pszProduct, "Meridian_02") )
        nProduct = NPC_MERIDIAN2;
    else if( STARTS_WITH_CI(pszProduct, "Meridian_01") )
        nProduct = NPC_MERIDIAN;
    else if( EQUAL(pszProduct,NTF_BOUNDARYLINE)
             && STARTS_WITH_CI(pszPVName, "A10N_FC") )
        nProduct = NPC_BOUNDARYLINE;
    else if( EQUAL(pszProduct,NTF_BOUNDARYLINE)
             && STARTS_WITH_CI(pszPVName, "A20N_FC") )
        nProduct = NPC_BL2000;
    else if( STARTS_WITH_CI(pszProduct, "BaseData.GB") )
        nProduct = NPC_BASEDATA;
    else if( STARTS_WITH_CI(pszProduct, "OSCAR_ASSET") )
        nProduct = NPC_OSCAR_ASSET;
    else if( STARTS_WITH_CI(pszProduct, "OSCAR_TRAFF") )
        nProduct = NPC_OSCAR_TRAFFIC;
    else if( STARTS_WITH_CI(pszProduct, "OSCAR_ROUTE") )
        nProduct = NPC_OSCAR_ROUTE;
    else if( STARTS_WITH_CI(pszProduct, "OSCAR_NETWO") )
        nProduct = NPC_OSCAR_NETWORK;
    else if( STARTS_WITH_CI(pszProduct, "ADDRESS_POI") )
        nProduct = NPC_ADDRESS_POINT;
    else if( STARTS_WITH_CI(pszProduct, "CODE_POINT") )
    {
        if( GetAttDesc( "RH" ) == NULL )
            nProduct = NPC_CODE_POINT;
        else
            nProduct = NPC_CODE_POINT_PLUS;
    }
    else if( STARTS_WITH_CI(pszProduct, "OS_LANDRANGER_DTM") )
        nProduct = NPC_LANDRANGER_DTM;
    else if( STARTS_WITH_CI(pszProduct, "L-F_PROFILE_DTM") )
        nProduct = NPC_LANDFORM_PROFILE_DTM;
    else if( STARTS_WITH_CI(pszProduct, "NEXTMap Britain DTM") )
        nProduct = NPC_LANDFORM_PROFILE_DTM; // Treat as landform

    if( poDS->GetOption("FORCE_GENERIC") != NULL
        && !EQUAL(poDS->GetOption("FORCE_GENERIC"),"OFF") )
        nProduct = NPC_UNKNOWN;

    // No point in caching lines if there are no polygons.
    if( nProduct != NPC_BOUNDARYLINE && nProduct != NPC_BL2000 )
        bCacheLines = FALSE;

/* -------------------------------------------------------------------- */
/*      Handle the section header record.                               */
/* -------------------------------------------------------------------- */
    nSavedFeatureId = nBaseFeatureId;
    nStartPos = VSIFTellL(fp);

    pszTileName = CPLStrdup(poRecord->GetField(3,12));        // SECT_REF
    while( pszTileName[strlen(pszTileName)-1] == ' ' )
        pszTileName[strlen(pszTileName)-1] = '\0';

    nCoordWidth = atoi(poRecord->GetField(15,19));            // XYLEN
    if( nCoordWidth == 0 )
        nCoordWidth = 10;

    nZWidth = atoi(poRecord->GetField(31,35));                // ZLEN
    if( nZWidth == 0 )
        nZWidth = 10;

    dfXYMult = atoi(poRecord->GetField(21,30)) / 1000.0;      // XY_MULT
    dfXOrigin = atoi(poRecord->GetField(47,56));
    dfYOrigin = atoi(poRecord->GetField(57,66));
    dfTileXSize = atoi(poRecord->GetField(23+74,32+74));
    dfTileYSize = atoi(poRecord->GetField(33+74,42+74));
    dfZMult = atoi(poRecord->GetField(37,46)) / 1000.0;

/* -------------------------------------------------------------------- */
/*      Setup scale and transformation factor for text height.          */
/* -------------------------------------------------------------------- */
    if( poRecord->GetLength() >= 187 )
        dfScale = atoi(poRecord->GetField(148+31,148+39));
    else if( nProduct == NPC_STRATEGI )
        dfScale = 250000;
    else if( nProduct == NPC_MERIDIAN || nProduct == NPC_MERIDIAN2 )
        dfScale = 100000;
    else if( nProduct == NPC_LANDFORM_PROFILE_CONT )
        dfScale = 10000;
    else if( nProduct == NPC_LANDRANGER_CONT )
        dfScale = 50000;
    else if( nProduct == NPC_OSCAR_ASSET
             || nProduct == NPC_OSCAR_TRAFFIC
             || nProduct == NPC_OSCAR_NETWORK
             || nProduct == NPC_OSCAR_ROUTE )
        dfScale = 10000;
    else if( nProduct == NPC_BASEDATA )
        dfScale = 625000;
    else /*if( nProduct == NPC_BOUNDARYLINE ) or default case */
        dfScale = 10000;

    if( dfScale != 0.0 )
        dfPaperToGround = dfScale / 1000.0;
    else
        dfPaperToGround = 0.0;

    delete poRecord;

/* -------------------------------------------------------------------- */
/*      Ensure we have appropriate layers defined.                      */
/* -------------------------------------------------------------------- */
    CPLErrorReset();

    if( !IsRasterProduct() )
        EstablishLayers();
    else
        EstablishRasterAccess();

    return CPLGetLastErrorType() != CE_Failure;
}