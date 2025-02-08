OGRErr OGRSpatialReference::importFromESRI( char **papszPrj )

{
    if( papszPrj == NULL || papszPrj[0] == NULL )
        return OGRERR_CORRUPT_DATA;

/* -------------------------------------------------------------------- */
/*      ArcGIS and related products now use a variant of Well Known     */
/*      Text.  Try to recognize this and ingest it.  WKT is usually     */
/*      all on one line, but we will accept multi-line formats and      */
/*      concatenate.                                                    */
/* -------------------------------------------------------------------- */
    if( STARTS_WITH_CI(papszPrj[0], "GEOGCS")
        || STARTS_WITH_CI(papszPrj[0], "PROJCS")
        || STARTS_WITH_CI(papszPrj[0], "LOCAL_CS") )
    {
        char *pszWKT = CPLStrdup(papszPrj[0]);
        for( int index = 1; papszPrj[index] != NULL; index++ )
        {
            pszWKT = static_cast<char *>(
                CPLRealloc(pszWKT, strlen(pszWKT)+strlen(papszPrj[index]) + 1));
            strcat( pszWKT, papszPrj[index] );
        }
        char *pszWKT2 = pszWKT;
        OGRErr eErr = importFromWkt( &pszWKT2 );
        CPLFree( pszWKT );

        if( eErr == OGRERR_NONE )
            eErr = morphFromESRI();
        return eErr;
    }

/* -------------------------------------------------------------------- */
/*      Operate on the basis of the projection name.                    */
/* -------------------------------------------------------------------- */
    CPLString osProj = OSR_GDS( papszPrj, "Projection", "" );

    if( EQUAL(osProj, "") )
    {
        CPLDebug( "OGR_ESRI", "Can't find Projection" );
        return OGRERR_CORRUPT_DATA;
    }
    else if( EQUAL(osProj, "GEOGRAPHIC") )
    {
        // Nothing to do.
    }
    else if( EQUAL(osProj, "utm") )
    {
        // <MASK>
    }
    else if( EQUAL(osProj, "STATEPLANE") )
    {
        int nZone = static_cast<int>( OSR_GDV( papszPrj, "zone", 0.0 ) );
        if( nZone != 0 )
            nZone = ESRIToUSGSZone( nZone );
        else
            nZone = static_cast<int>( OSR_GDV( papszPrj, "fipszone", 0.0 ) );

        if( nZone != 0 )
        {
            if( EQUAL(OSR_GDS( papszPrj, "Datum", "NAD83"), "NAD27") )
                SetStatePlane( nZone, FALSE );
            else
                SetStatePlane( nZone, TRUE );
        }
    }
    else if( EQUAL(osProj, "GREATBRITIAN_GRID")
             || EQUAL(osProj, "GREATBRITAIN_GRID") )
    {
        const char *pszWkt =
            "PROJCS[\"OSGB 1936 / British National Grid\","
            "GEOGCS[\"OSGB 1936\",DATUM[\"OSGB_1936\","
            "SPHEROID[\"Airy 1830\",6377563.396,299.3249646]],"
            "PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0.0174532925199433]],"
            "PROJECTION[\"Transverse_Mercator\"],"
            "PARAMETER[\"latitude_of_origin\",49],"
            "PARAMETER[\"central_meridian\",-2],"
            "PARAMETER[\"scale_factor\",0.999601272],"
            "PARAMETER[\"false_easting\",400000],"
            "PARAMETER[\"false_northing\",-100000],UNIT[\"metre\",1]]";

        importFromWkt( const_cast<char **>(&pszWkt) );
    }
    else if( EQUAL(osProj, "ALBERS") )
    {
        SetACEA( OSR_GDV( papszPrj, "PARAM_1", 0.0 ),
                 OSR_GDV( papszPrj, "PARAM_2", 0.0 ),
                 OSR_GDV( papszPrj, "PARAM_4", 0.0 ),
                 OSR_GDV( papszPrj, "PARAM_3", 0.0 ),
                 OSR_GDV( papszPrj, "PARAM_5", 0.0 ),
                 OSR_GDV( papszPrj, "PARAM_6", 0.0 ) );
    }
    else if( EQUAL(osProj, "LAMBERT") )
    {
        SetLCC( OSR_GDV( papszPrj, "PARAM_1", 0.0 ),
                OSR_GDV( papszPrj, "PARAM_2", 0.0 ),
                OSR_GDV( papszPrj, "PARAM_4", 0.0 ),
                OSR_GDV( papszPrj, "PARAM_3", 0.0 ),
                OSR_GDV( papszPrj, "PARAM_5", 0.0 ),
                OSR_GDV( papszPrj, "PARAM_6", 0.0 ) );
    }
    else if( EQUAL(osProj, "LAMBERT_AZIMUTHAL") )
    {
        SetLAEA( OSR_GDV( papszPrj, "PARAM_2", 0.0 ),
                 OSR_GDV( papszPrj, "PARAM_1", 0.0 ),
                 OSR_GDV( papszPrj, "PARAM_3", 0.0 ),
                 OSR_GDV( papszPrj, "PARAM_4", 0.0 ) );
    }
    else if( EQUAL(osProj, "EQUIDISTANT_CONIC") )
    {
        const int nStdPCount = static_cast<int>(
            OSR_GDV( papszPrj, "PARAM_1", 0.0 ) );

        if( nStdPCount == 1 )
        {
            SetEC( OSR_GDV( papszPrj, "PARAM_2", 0.0 ),
                   OSR_GDV( papszPrj, "PARAM_2", 0.0 ),
                   OSR_GDV( papszPrj, "PARAM_4", 0.0 ),
                   OSR_GDV( papszPrj, "PARAM_3", 0.0 ),
                   OSR_GDV( papszPrj, "PARAM_5", 0.0 ),
                   OSR_GDV( papszPrj, "PARAM_6", 0.0 ) );
        }
        else
        {
            SetEC( OSR_GDV( papszPrj, "PARAM_2", 0.0 ),
                   OSR_GDV( papszPrj, "PARAM_3", 0.0 ),
                   OSR_GDV( papszPrj, "PARAM_5", 0.0 ),
                   OSR_GDV( papszPrj, "PARAM_4", 0.0 ),
                   OSR_GDV( papszPrj, "PARAM_5", 0.0 ),
                   OSR_GDV( papszPrj, "PARAM_7", 0.0 ) );
        }
    }
    else if( EQUAL(osProj, "TRANSVERSE") )
    {
        SetTM( OSR_GDV( papszPrj, "PARAM_3", 0.0 ),
               OSR_GDV( papszPrj, "PARAM_2", 0.0 ),
               OSR_GDV( papszPrj, "PARAM_1", 0.0 ),
               OSR_GDV( papszPrj, "PARAM_4", 0.0 ),
               OSR_GDV( papszPrj, "PARAM_5", 0.0 ) );
    }
    else if( EQUAL(osProj, "POLAR") )
    {
        SetPS( OSR_GDV( papszPrj, "PARAM_2", 0.0 ),
               OSR_GDV( papszPrj, "PARAM_1", 0.0 ),
               1.0,
               OSR_GDV( papszPrj, "PARAM_3", 0.0 ),
               OSR_GDV( papszPrj, "PARAM_4", 0.0 ) );
    }
    else if( EQUAL(osProj, "MERCATOR") )
    {
        SetMercator( OSR_GDV( papszPrj, "PARAM_2", 0.0 ),
                     OSR_GDV( papszPrj, "PARAM_1", 0.0 ),
                     1.0,
                     OSR_GDV( papszPrj, "PARAM_3", 0.0 ),
                     OSR_GDV( papszPrj, "PARAM_4", 0.0 ) );
    }
    else if( EQUAL(osProj, SRS_PT_MERCATOR_AUXILIARY_SPHERE) )
    {
       // This is EPSG:3875 Pseudo Mercator. We might as well import it from
       // the EPSG spec.
       importFromEPSG(3857);
    }
    else if( EQUAL(osProj, "POLYCONIC") )
    {
        SetPolyconic( OSR_GDV( papszPrj, "PARAM_2", 0.0 ),
                      OSR_GDV( papszPrj, "PARAM_1", 0.0 ),
                      OSR_GDV( papszPrj, "PARAM_3", 0.0 ),
                      OSR_GDV( papszPrj, "PARAM_4", 0.0 ) );
    }
    else
    {
        CPLDebug( "OGR_ESRI", "Unsupported projection: %s", osProj.c_str() );
        SetLocalCS( osProj );
    }

/* -------------------------------------------------------------------- */
/*      Try to translate the datum/spheroid.                            */
/* -------------------------------------------------------------------- */
    if( !IsLocal() && GetAttrNode( "GEOGCS" ) == NULL )
    {
        const CPLString osDatum = OSR_GDS( papszPrj, "Datum", "");

        if( EQUAL(osDatum, "NAD27") || EQUAL(osDatum, "NAD83")
            || EQUAL(osDatum, "WGS84") || EQUAL(osDatum, "WGS72") )
        {
            SetWellKnownGeogCS( osDatum );
        }
        else if( EQUAL( osDatum, "EUR" )
                 || EQUAL( osDatum, "ED50" ) )
        {
            SetWellKnownGeogCS( "EPSG:4230" );
        }
        else if( EQUAL( osDatum, "GDA94" ) )
        {
            SetWellKnownGeogCS( "EPSG:4283" );
        }
        else
        {
            CPLString osSpheroid = OSR_GDS( papszPrj, "Spheroid", "");

            if( EQUAL(osSpheroid, "INT1909")
                || EQUAL(osSpheroid, "INTERNATIONAL1909") )
            {
                OGRSpatialReference oGCS;
                oGCS.importFromEPSG( 4022 );
                CopyGeogCSFrom( &oGCS );
            }
            else if( EQUAL(osSpheroid, "AIRY") )
            {
                OGRSpatialReference oGCS;
                oGCS.importFromEPSG( 4001 );
                CopyGeogCSFrom( &oGCS );
            }
            else if( EQUAL(osSpheroid, "CLARKE1866") )
            {
                OGRSpatialReference oGCS;
                oGCS.importFromEPSG( 4008 );
                CopyGeogCSFrom( &oGCS );
            }
            else if( EQUAL(osSpheroid, "GRS80") )
            {
                OGRSpatialReference oGCS;
                oGCS.importFromEPSG( 4019 );
                CopyGeogCSFrom( &oGCS );
            }
            else if( EQUAL(osSpheroid, "KRASOVSKY")
                     || EQUAL(osSpheroid, "KRASSOVSKY")
                     || EQUAL(osSpheroid, "KRASSOWSKY") )
            {
                OGRSpatialReference oGCS;
                oGCS.importFromEPSG( 4024 );
                CopyGeogCSFrom( &oGCS );
            }
            else if( EQUAL(osSpheroid, "Bessel") )
            {
                OGRSpatialReference oGCS;
                oGCS.importFromEPSG( 4004 );
                CopyGeogCSFrom( &oGCS );
            }
            else
            {
                bool bFoundParameters = false;
                for( int iLine = 0; papszPrj[iLine] != NULL; iLine++ )
                {
                    if( STARTS_WITH_CI(papszPrj[iLine], "Parameters") )
                    {
                        char** papszTokens =
                            CSLTokenizeString(papszPrj[iLine] +
                                              strlen("Parameters"));
                        if( CSLCount(papszTokens) == 2 )
                        {
                            OGRSpatialReference oGCS;
                            const double dfSemiMajor = CPLAtof(papszTokens[0]);
                            const double dfSemiMinor = CPLAtof(papszTokens[1]);
                            const double dfInvFlattening =
                                OSRCalcInvFlattening(dfSemiMajor, dfSemiMinor);
                            oGCS.SetGeogCS( "unknown", "unknown", "unknown",
                                            dfSemiMajor, dfInvFlattening );
                            CopyGeogCSFrom( &oGCS );
                            bFoundParameters = true;
                        }
                        CSLDestroy(papszTokens);
                        break;
                    }
                }
                if( !bFoundParameters )
                {
                    // If unknown, default to WGS84 so there is something there.
                    SetWellKnownGeogCS( "WGS84" );
                }
            }
        }
    }

/* -------------------------------------------------------------------- */
/*      Linear units translation                                        */
/* -------------------------------------------------------------------- */
    if( IsLocal() || IsProjected() )
    {
        const double dfOldUnits = GetLinearUnits();
        const CPLString osValue = OSR_GDS( papszPrj, "Units", "" );

        if( EQUAL(osValue, "" ) )
            SetLinearUnitsAndUpdateParameters( SRS_UL_METER, 1.0 );
        else if( EQUAL(osValue, "FEET") )
            SetLinearUnitsAndUpdateParameters( SRS_UL_US_FOOT,
                                               CPLAtof(SRS_UL_US_FOOT_CONV) );
        else if( CPLAtof(osValue) != 0.0 )
            SetLinearUnitsAndUpdateParameters( "user-defined",
                                               1.0 / CPLAtof(osValue) );
        else
            SetLinearUnitsAndUpdateParameters( osValue, 1.0 );

        // If we have reset the linear units we should clear any authority
        // nodes on the PROJCS.  This especially applies to state plane
        // per bug #1697.
        const double dfNewUnits = GetLinearUnits();
        if( dfOldUnits != 0.0
            && (dfNewUnits / dfOldUnits < 0.9999999
                || dfNewUnits / dfOldUnits > 1.0000001) )
        {
            if( GetRoot()->FindChild( "AUTHORITY" ) != -1 )
                GetRoot()->DestroyChild(GetRoot()->FindChild( "AUTHORITY" ));
        }
    }

    return OGRERR_NONE;
}