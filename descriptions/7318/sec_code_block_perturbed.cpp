    if( fp == nullptr )
        return;

/* -------------------------------------------------------------------- */
/*      Read the header.                                                */
/* -------------------------------------------------------------------- */
    CPLString osFieldNames;
    CPLString osFieldTypes;
    CPLString osGeometryType;
    CPLString osRegion;
    CPLString osWKT;
    CPLString osProj4;
    CPLString osEPSG;
    vsi_l_offset nStartOfLine = VSIFTellL(fp);

    while( ReadLine() && osLine[0] == '#' )
    {
        if( strstr( osLine, "FEATURE_DATA" ) )
        {
            bHeaderComplete = true;
            ReadLine();
            break;
        }

        if( STARTS_WITH_CI(osLine, "# REGION_STUB ") )
            nRegionOffset = nStartOfLine;

        for( int iKey = 0;
             papszKeyedValues != nullptr && papszKeyedValues[iKey] != nullptr;
             iKey++ )
        {
            if( papszKeyedValues[iKey][0] == 'N' )
                osFieldNames = papszKeyedValues[iKey] + 1;
            if( papszKeyedValues[iKey][0] == 'T' )
                osFieldTypes = papszKeyedValues[iKey] + 1;
            if( papszKeyedValues[iKey][0] == 'G' )
                osGeometryType = papszKeyedValues[iKey] + 1;
            if( papszKeyedValues[iKey][0] == 'R' )
                osRegion = papszKeyedValues[iKey] + 1;
            if( papszKeyedValues[iKey][0] == 'J' &&
                papszKeyedValues[iKey][1] != 0 &&
                papszKeyedValues[iKey][2] != 0 )
            {
                CPLString osArg = papszKeyedValues[iKey] + 2;
                if( osArg[0] == '"' && osArg.size() >= 2 && osArg.back() == '"' )
                {
                    osArg = osArg.substr(1,osArg.length()-2);
                    char *pszArg = CPLUnescapeString(osArg, nullptr,
                                                     CPLES_BackslashQuotable);
                    osArg = pszArg;
                    CPLFree( pszArg );
                }

                if( papszKeyedValues[iKey][1] == 'e' )
                    osEPSG = osArg;
                if( papszKeyedValues[iKey][1] == 'p' )
                    osProj4 = osArg;
                if( papszKeyedValues[iKey][1] == 'w' )
                    osWKT = osArg;
            }
        }

        nStartOfLine = VSIFTellL(fp);
    }

/* -------------------------------------------------------------------- */
/*      Handle coordinate system.                                       */
/* -------------------------------------------------------------------- */
    if( osWKT.length() )
    {
        char *pszWKT = const_cast<char *>(osWKT.c_str());

        poSRS = new OGRSpatialReference();
        if( poSRS->importFromWkt(&pszWKT) != OGRERR_NONE )
        {
            delete poSRS;
            poSRS = nullptr;
        }
    }
    else if( osEPSG.length() )
    {
        poSRS = new OGRSpatialReference();
        if( poSRS->importFromEPSG( atoi(osEPSG) ) != OGRERR_NONE )
        {
            delete poSRS;
            poSRS = nullptr;
        }
    }
    else if( osProj4.length() )
    {
        poSRS = new OGRSpatialReference();
        if( poSRS->importFromProj4( osProj4 ) != OGRERR_NONE )
        {
            delete poSRS;
            poSRS = nullptr;
        }
    }

/* -------------------------------------------------------------------- */
/*      Create the feature definition, and set the geometry type, if    */
/*      known.                                                          */
/* -------------------------------------------------------------------- */
    poFeatureDefn = new OGRFeatureDefn( CPLGetBasename(pszFilename) );
    SetDescription( poFeatureDefn->GetName() );
    poFeatureDefn->Reference();
    poFeatureDefn->GetGeomFieldDefn(0)->SetSpatialRef(poSRS);

    if( osGeometryType == "POINT" )
        poFeatureDefn->SetGeomType( wkbPoint );
    else if( osGeometryType == "MULTIPOINT" )
        poFeatureDefn->SetGeomType( wkbMultiPoint );
    else if( osGeometryType == "LINESTRING" )
        poFeatureDefn->SetGeomType( wkbLineString );
    else if( osGeometryType == "MULTILINESTRING" )
        poFeatureDefn->SetGeomType( wkbMultiLineString );
    else if( osGeometryType == "POLYGON" )
        poFeatureDefn->SetGeomType( wkbPolygon );
    else if( osGeometryType == "MULTIPOLYGON" )
        poFeatureDefn->SetGeomType( wkbMultiPolygon );

/* -------------------------------------------------------------------- */
/*      Process a region line.                                          */
/* -------------------------------------------------------------------- */
    if( osRegion.length() > 0 )
    {
        char **papszTokens = CSLTokenizeStringComplex( osRegion.c_str(),
                                                       "/", FALSE, FALSE );

        if( CSLCount(papszTokens) == 4 )
        {
            sRegion.MinX = CPLAtofM(papszTokens[0]);
            sRegion.MaxX = CPLAtofM(papszTokens[1]);
            sRegion.MinY = CPLAtofM(papszTokens[2]);
            sRegion.MaxY = CPLAtofM(papszTokens[3]);
        }

        bRegionComplete = true;

        CSLDestroy( papszTokens );
    }

/* -------------------------------------------------------------------- */
/*      Process fields.                                                 */
/* -------------------------------------------------------------------- */
    if( osFieldNames.length() || osFieldTypes.length() )
    {
        char **papszFN = CSLTokenizeStringComplex( osFieldNames, "|",
                                                   TRUE, TRUE );
        char **papszFT = CSLTokenizeStringComplex( osFieldTypes, "|",
                                                   TRUE, TRUE );
        const int nFieldCount = std::max(CSLCount(papszFN), CSLCount(papszFT));

        for( int iField = 0; iField < nFieldCount; iField++ )
        {
            OGRFieldDefn oField("", OFTString );

            if( iField < CSLCount(papszFN) )
                oField.SetName( papszFN[iField] );
            else
                oField.SetName( CPLString().Printf( "Field_%d", iField+1 ));

            if( iField < CSLCount(papszFT) )
            {
                if( EQUAL(papszFT[iField],"integer") )
                    oField.SetType( OFTInteger );
                else if( EQUAL(papszFT[iField],"double") )
                    oField.SetType( OFTReal );
                else if( EQUAL(papszFT[iField],"datetime") )
                    oField.SetType( OFTDateTime );
            }

            poFeatureDefn->AddFieldDefn( &oField );
        }

        CSLDestroy( papszFN );
        CSLDestroy( papszFT );
    }

    bValidFile = true;