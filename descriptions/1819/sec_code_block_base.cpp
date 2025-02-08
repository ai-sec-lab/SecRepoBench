/* -------------------------------------------------------------------- */
/*      Handle feature class name records.                              */
/* -------------------------------------------------------------------- */
        if( poRecord->GetType() == NRT_FCR && poRecord->GetLength() >= 37 )
        {
            nFCCount++;

            papszFCNum = CSLAddString( papszFCNum, poRecord->GetField(3,6) );

            CPLString osFCName;
            const char *pszData = poRecord->GetData();

            // CODE_COM
            int iChar = 15;
            for( ; pszData[iChar] == ' ' && iChar > 5; iChar-- ) {}

            if( iChar > 6 )
                osFCName += poRecord->GetField(7,iChar+1);

            // STCLASS
            for( iChar = 35; pszData[iChar] == ' ' && iChar > 15; iChar-- ) {}

            if( iChar > 15 )
            {
                if( !osFCName.empty() )
                    osFCName += " : " ;
                osFCName += poRecord->GetField(17,iChar+1);
            }

            // FEATDES
            for( iChar = 36;
                 pszData[iChar] != '\0' && pszData[iChar] != '\\';
                 iChar++ ) {}

            if( iChar > 37 )
            {
                if( !osFCName.empty() )
                    osFCName += " : " ;
                osFCName += poRecord->GetField(37,iChar);
            }

            papszFCName = CSLAddString(papszFCName, osFCName );
        }

/* -------------------------------------------------------------------- */
/*      Handle attribute description records.                           */
/* -------------------------------------------------------------------- */
        else if( poRecord->GetType() == NRT_ADR )
        {
            nAttCount++;

            pasAttDesc = static_cast<NTFAttDesc *>(
                CPLRealloc( pasAttDesc, sizeof(NTFAttDesc) * nAttCount ));

            ProcessAttDesc( poRecord, pasAttDesc + nAttCount - 1 );
        }

/* -------------------------------------------------------------------- */
/*      Handle attribute description records.                           */
/* -------------------------------------------------------------------- */
        else if( poRecord->GetType() == NRT_CODELIST )
        {
            NTFCodeList *poCodeList = new NTFCodeList( poRecord );
            NTFAttDesc  *psAttDesc = GetAttDesc( poCodeList->szValType );
            if( psAttDesc == NULL )
            {
                CPLDebug( "NTF", "Got CODELIST for %s without ATTDESC.",
                          poCodeList->szValType );
                delete poCodeList;
            }
            else
            {
                psAttDesc->poCodeList = poCodeList;
            }
        }

/* -------------------------------------------------------------------- */
/*      Handle database header record.                                  */
/* -------------------------------------------------------------------- */
        else if( poRecord->GetType() == NRT_DHR && pszProduct == NULL )
        {
            pszProduct = CPLStrdup(poRecord->GetField(3,22));
            for( int iChar = static_cast<int>(strlen(pszProduct))-1;
                 iChar > 0 && pszProduct[iChar] == ' ';
                 pszProduct[iChar--] = '\0' ) {}

            pszPVName = CPLStrdup(poRecord->GetField(76+3,76+22));
            for( int iChar = static_cast<int>(strlen(pszPVName))-1;
                 iChar > 0 && pszPVName[iChar] == ' ';
                 pszPVName[iChar--] = '\0' ) {}
        }