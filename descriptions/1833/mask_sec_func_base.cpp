void OGRNTFDataSource::WorkupGeneric( NTFFileReader * poReader )

{
    NTFRecord   **papoGroup = NULL;

    if( poReader->GetNTFLevel() > 2 )
    {
        poReader->IndexFile();
        if( CPLGetLastErrorType() == CE_Failure )
            return;
    }
    else
        poReader->Reset();

/* ==================================================================== */
/*      Read all record groups in the file.                             */
/* ==================================================================== */
    while( true )
    {
/* -------------------------------------------------------------------- */
/*      Read a record group                                             */
/* -------------------------------------------------------------------- */
        if( poReader->GetNTFLevel() > 2 )
            papoGroup = poReader->GetNextIndexedRecordGroup(papoGroup);
        else
            papoGroup = poReader->ReadRecordGroup();

        // <MASK>
    }

    if( GetOption("CACHING") != NULL
        && EQUAL(GetOption("CACHING"),"OFF") )
        poReader->DestroyIndex();

    poReader->Reset();
}