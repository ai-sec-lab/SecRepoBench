SDTSAttrRecord *poAttrRecord = reinterpret_cast<SDTSAttrRecord *>(
        GetIndexedFeatureRef( poModId ) );

    if( poAttrRecord == NULL )
        return NULL;

    return poAttrRecord->poATTR;