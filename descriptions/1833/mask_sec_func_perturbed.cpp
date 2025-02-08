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

        // <MASK>
    }

    if( GetOption("CACHING") != NULL
        && EQUAL(GetOption("CACHING"),"OFF") )
        pReader->DestroyIndex();

    pReader->Reset();
}