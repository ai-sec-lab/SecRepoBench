void SDTSFeature::ApplyATID( DDFField * poField )

{
    DDFSubfieldDefn *poMODN
        = poField->GetFieldDefn()->FindSubfieldDefn( "MODN" );
    if( poMODN == NULL )
    {
        // CPLAssert( false );
        return;
    }

    bool bUsualFormat = poMODN->GetWidth() == 4;
    const int repeatcount = poField->GetRepeatCount();
    for( int iRepeat = 0; iRepeat < repeatcount; iRepeat++ )
    {
        paoATID = reinterpret_cast<SDTSModId *>(
          CPLRealloc( paoATID, sizeof(SDTSModId)*(nAttributes+1) ) );

        SDTSModId *poModId = paoATID + nAttributes;
        memset(poModId, 0, sizeof(SDTSModId));

        if( bUsualFormat )
        {
            const char * pabyData
                = poField->GetSubfieldData( poMODN, NULL, iRepeat );
            if( strlen(pabyData) < 5 )
                return;

            memcpy( poModId->szModule, pabyData, 4 );
            poModId->szModule[4] = '\0';
            poModId->nRecord = atoi(pabyData + 4);
            poModId->szOBRP[0] = '\0';
        }
        else
        {
            poModId->Set( poField );
        }

        nAttributes++;
    }
}