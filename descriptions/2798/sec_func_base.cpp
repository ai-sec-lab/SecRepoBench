DDFField *SDTSTransfer::GetAttr( SDTSModId *poModId )

{
    SDTSAttrRecord *poAttrRecord = dynamic_cast<SDTSAttrRecord *>(
        GetIndexedFeatureRef( poModId ) );

    if( poAttrRecord == NULL )
        return NULL;

    return poAttrRecord->poATTR;
}