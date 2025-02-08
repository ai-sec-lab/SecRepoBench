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
    const int nRepeatCount = poField->GetRepeatCount();
    for( int iRepeat = 0; iRepeat < nRepeatCount; iRepeat++ )
    {
        paoATID = reinterpret_cast<SDTSModId *>(
          CPLRealloc( paoATID, sizeof(SDTSModId)*(nAttributes+1) ) );

        SDTSModId *poModId = paoATID + nAttributes;
        memset(poModId, 0, sizeof(SDTSModId));

        if( bUsualFormat )
        {
            const char * pabyData
                = poField->GetSubfieldData( poMODN, NULL, iRepeat );
            // <MASK>
        }
        else
        {
            poModId->Set( poField );
        }

        nAttributes++;
    }
}