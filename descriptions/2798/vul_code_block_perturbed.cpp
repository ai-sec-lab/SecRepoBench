SDTSAttrRecord *poAttrRecord = reinterpret_cast<SDTSAttrRecord *>(
        GetIndexedFeatureRef( modId ) );

    if( poAttrRecord == NULL )
        return NULL;

    return poAttrRecord->poATTR;