void OGRNTFDataSource::WorkupGeneric( NTFFileReader * pReader )

{
    NTFRecord   **papoGroup = NULL;

    if( pReader->GetNTFLevel() > 2 )
    {
        pReader->IndexFile();
        if( CPLGetLastErrorType() == CE_Failure )
            return;
    }
    else
        pReader->Reset();

/* ==================================================================== */
/*      Read all record groups in the file.                             */
/* ==================================================================== */
    while( true )
    {
/* -------------------------------------------------------------------- */
/*      Read a record group                                             */
/* -------------------------------------------------------------------- */
        if( pReader->GetNTFLevel() > 2 )
            papoGroup = pReader->GetNextIndexedRecordGroup(papoGroup);
        else
            papoGroup = pReader->ReadRecordGroup();

        if( papoGroup == NULL ||
            papoGroup[0]->GetType() < 0 ||
            papoGroup[0]->GetType() >= 99 )
            break;

/* -------------------------------------------------------------------- */
/*      Get the class corresponding to the anchor record.               */
/* -------------------------------------------------------------------- */
        NTFGenericClass *poClass = GetGClass( papoGroup[0]->GetType() );
        char           **papszFullAttList = NULL;

        poClass->nFeatureCount++;

/* -------------------------------------------------------------------- */
/*      Loop over constituent records collecting attributes.            */
/* -------------------------------------------------------------------- */
        for( int iRec = 0; papoGroup[iRec] != NULL; iRec++ )
        {
            NTFRecord   *poRecord = papoGroup[iRec];

            switch( poRecord->GetType() )
            {
              case NRT_ATTREC:
              {
                  char  **papszTypes, **papszValues;

                  pReader->ProcessAttRec( poRecord, NULL,
                                           &papszTypes, &papszValues );

                  for( int iAtt = 0; papszTypes != NULL &&
                                     papszTypes[iAtt] != NULL; iAtt++ )
                  {
                      NTFAttDesc *poAttDesc =
                          pReader->GetAttDesc( papszTypes[iAtt] );
                      if( poAttDesc != NULL )
                      {
                          poClass->CheckAddAttr( poAttDesc->val_type,
                                                 poAttDesc->finter,
                                                 static_cast<int>(strlen(papszValues[iAtt])) );
                      }

                      if( CSLFindString( papszFullAttList,
                                         papszTypes[iAtt] ) == -1 )
                          papszFullAttList =
                              CSLAddString( papszFullAttList,
                                            papszTypes[iAtt] );
                      else if( poAttDesc != NULL )
                          poClass->SetMultiple( poAttDesc->val_type );
                  }

                  CSLDestroy( papszTypes );
                  CSLDestroy( papszValues );
              }
              break;

              case NRT_TEXTREP:
              case NRT_NAMEPOSTN:
                poClass->CheckAddAttr( "FONT", "I4", 4 );
                poClass->CheckAddAttr( "TEXT_HT", "R3,1", 3 );
                poClass->CheckAddAttr( "TEXT_HT_GROUND", "R9,3", 9 );
                poClass->CheckAddAttr( "TEXT_HT", "R3,1", 3 );
                poClass->CheckAddAttr( "DIG_POSTN", "I1", 1 );
                poClass->CheckAddAttr( "ORIENT", "R4,1", 4 );
                break;

              case NRT_NAMEREC:
                poClass->CheckAddAttr( "TEXT", "A*",
                                       atoi(poRecord->GetField(13,14)) );
                break;

              case NRT_GEOMETRY:
              case NRT_GEOMETRY3D:
                  if( atoi(poRecord->GetField(3,8)) != 0 )
                      poClass->CheckAddAttr( "GEOM_ID", "I6", 6 );
                  if( poRecord->GetType() == NRT_GEOMETRY3D )
                      poClass->b3D = TRUE;
                  break;

              case NRT_POINTREC:
              case NRT_LINEREC:
                if( pReader->GetNTFLevel() < 3 )
                {
                    NTFAttDesc *poAttDesc =
                        pReader->GetAttDesc(poRecord->GetField(9,10));
                    if( poAttDesc != NULL )
                        poClass->CheckAddAttr( poAttDesc->val_type,
                                               poAttDesc->finter, 6 );

                    if( !EQUAL(poRecord->GetField(17,20),"    ") )
                        poClass->CheckAddAttr( "FEAT_CODE", "A4", 4 );
                }
                break;

              default:
                break;
            }
        }

        CSLDestroy( papszFullAttList );
    }

    if( GetOption("CACHING") != NULL
        && EQUAL(GetOption("CACHING"),"OFF") )
        pReader->DestroyIndex();

    pReader->Reset();
}