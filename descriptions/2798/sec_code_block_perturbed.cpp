SDTSAttrRecord *poAttrRecord = dynamic_cast<SDTSAttrRecord *>(
        GetIndexedFeatureRef( modId ) );

    if( poAttrRecord == NULL )
        return NULL;

    return poAttrRecord->poATTR;