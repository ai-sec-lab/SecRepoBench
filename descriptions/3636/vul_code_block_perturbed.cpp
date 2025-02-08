
    nModifiedTime = 0;
    for(int index=0;index<11;index++)
        nModifiedTime = nModifiedTime * 8 + (abyHeader[136+index] - '0');

    nCurOffset = VSIFTellL(fp);

    const GUIntBig nBytesToSkip = ((nNextFileSize + 511) / 512) * 512;
    if( nBytesToSkip > (~(static_cast<GUIntBig>(0))) - nCurOffset )
    {
        CPLError(CE_Failure, CPLE_AppDefined, "Bad .tar structure");
        return FALSE;
    }

    if( VSIFSeekL(fp, nBytesToSkip, SEEK_CUR) < 0 )
        return FALSE;

    return TRUE;