if( pszProjection != NULL && EQUAL(pszProjection, SRS_PT_MERCATOR_2SP) )
    {
        SetNode( "PROJCS|PROJECTION", "Mercator" );
    }

    // See #4861.
    else if( pszProjection != NULL &&
             EQUAL(pszProjection, SRS_PT_MERCATOR_1SP) )
    {
        SetNode( "PROJCS|PROJECTION", "Mercator" );

        const double dfK0 = GetNormProjParm(SRS_PP_SCALE_FACTOR, 1.0);

        const double dfInvFlattening = GetInvFlattening();
        double e2 = 0.0;
        if( dfInvFlattening != 0.0 )
        {
            const double f = 1.0 / dfInvFlattening;
            e2 = 2 * f - f * f;
        }
        const double dfStdP1Lat =
            acos( sqrt( (1.0 - e2) / ((1.0 / (dfK0 * dfK0)) - e2)) ) /
            M_PI * 180.0;
        if( poProjCS )
        {
            const int iScaleFactorChild =
                FindProjParm( SRS_PP_SCALE_FACTOR, poProjCS );
            if( iScaleFactorChild != -1 )
                poProjCS->DestroyChild( iScaleFactorChild);
            SetProjParm(SRS_PP_STANDARD_PARALLEL_1, dfStdP1Lat);
            FixupOrdering();
        }
    }

/* -------------------------------------------------------------------- */
/*      Convert SPHEROID name to use underscores instead of spaces.     */
/* -------------------------------------------------------------------- */
    OGR_SRSNode *poSpheroidChild = NULL;
    OGR_SRSNode *poSpheroid = GetAttrNode( "SPHEROID" );
    if( poSpheroid != NULL )
        poSpheroidChild = poSpheroid->GetChild(0);

    if( poSpheroidChild != NULL )
    {
        // char *pszNewValue =
        //     CPLStrdup(RemapSpheroidName(poSpheroidChild->GetValue()));
        char *pszNewValue = CPLStrdup(poSpheroidChild->GetValue());

        MorphNameToESRI( &pszNewValue );

        poSpheroidChild->SetValue( pszNewValue );
        CPLFree( pszNewValue );

        GetRoot()->applyRemapper("SPHEROID",
                                 const_cast<char **>(apszSpheroidMapping + 0),
                                 const_cast<char **>(apszSpheroidMapping + 1),
                                 2);
    }

    if( poSpheroid != NULL )
        poSpheroidChild = poSpheroid->GetChild(2);

    if( poSpheroidChild != NULL )
    {
      const char * pszValue = poSpheroidChild->GetValue();
      for( int i = 0; apszInvFlatteningMapping[i] != NULL; i += 2 )
      {
        if( EQUALN(apszInvFlatteningMapping[i], pszValue,
                   strlen(apszInvFlatteningMapping[i]) ))
        {
          poSpheroidChild->SetValue( apszInvFlatteningMapping[i+1] );
          break;
        }
      }
    }

/* -------------------------------------------------------------------- */
/*      Try to insert a D_ in front of the datum name.                  */
/* -------------------------------------------------------------------- */
    OGR_SRSNode *poDatum = GetAttrNode( "DATUM" );
    if( poDatum != NULL )
        poDatum = poDatum->GetChild(0);

    if( poDatum != NULL )
    {
        const char* pszDatumName = poDatum->GetValue();
        if( !STARTS_WITH_CI(pszDatumName, "D_") )
        {
            char *pszNewValue = static_cast<char *>(
                CPLMalloc(strlen(poDatum->GetValue())+3) );
            strcpy( pszNewValue, "D_" );
            strcat( pszNewValue, poDatum->GetValue() );
            poDatum->SetValue( pszNewValue );
            CPLFree( pszNewValue );
        }
    }

/* -------------------------------------------------------------------- */
/*                        final check names                             */
/* -------------------------------------------------------------------- */
    if( poProjCSNodeChild )
        pszProjCSName = poProjCSNodeChild->GetValue();

    if( pszProjCSName )
    {
      const char *pszGcsName = GetAttrValue( "GEOGCS" );
      if( pszGcsName && !STARTS_WITH_CI(pszGcsName, "GCS_") )
      {
        char* newGcsName = static_cast<char *>(
            CPLMalloc(strlen(pszGcsName) + 5) );
        strcpy( newGcsName, "GCS_" );
        strcat(newGcsName, pszGcsName);
        SetNewName( this, "GEOGCS", newGcsName );
        CPLFree( newGcsName );
        pszGcsName = GetAttrValue( "GEOGCS" );
      }
      RemapGeogCSName(this, pszGcsName);

      // Specific processing and remapping
      pszProjection = GetAttrValue("PROJECTION");
      if( pszProjection )
      {
        if( EQUAL(pszProjection, "Lambert_Conformal_Conic") )
        {
          if( FindProjParm( SRS_PP_STANDARD_PARALLEL_2, poProjCS ) < 0 )
          {
            int iChild = FindProjParm( SRS_PP_LATITUDE_OF_ORIGIN, poProjCS );
            int iChild1 = FindProjParm( SRS_PP_STANDARD_PARALLEL_1, poProjCS );
            if( iChild >= 0 && iChild1 < 0 )
            {
              const OGR_SRSNode *poParameter = poProjCS->GetChild(iChild);
              if( poParameter )
              {
                OGR_SRSNode *poNewParm = new OGR_SRSNode( "PARAMETER" );
                poNewParm->AddChild( new OGR_SRSNode( "standard_parallel_1" ) );
                poNewParm->AddChild( new OGR_SRSNode(
                    poParameter->GetChild(1)->GetValue() ) );
                poProjCS->AddChild( poNewParm );
              }
            }
          }
        }

        if( EQUAL(pszProjection, "Plate_Carree") )
        {
          int iChild = FindProjParm( SRS_PP_STANDARD_PARALLEL_1, poProjCS );
          if( iChild < 0 )
            iChild = FindProjParm( SRS_PP_PSEUDO_STD_PARALLEL_1, poProjCS );

          if( iChild >= 0 )
          {
            const OGR_SRSNode *poParameter = poProjCS->GetChild(iChild);
            if( !EQUAL(poParameter->GetChild(1)->GetValue(), "0.0") &&
                !EQUAL(poParameter->GetChild(1)->GetValue(), "0") )
            {
              SetNode( "PROJCS|PROJECTION", "Equidistant_Cylindrical" );
              pszProjection = GetAttrValue("PROJECTION");
            }
          }
        }

        DeleteParamBasedOnPrjName(
            this, pszProjection,
            const_cast<char **>(apszDeleteParametersBasedOnProjection));
        AddParamBasedOnPrjName(
            this, pszProjection,
            const_cast<char **>(apszAddParametersBasedOnProjection));
        RemapPValuesBasedOnProjCSAndPName(
            this, pszProjection,
            const_cast<char **>(apszParamValueMapping));
        RemapPNamesBasedOnProjCSAndPName(
            this, pszProjection,
            const_cast<char **>(apszParamNameMapping));
      }
    }