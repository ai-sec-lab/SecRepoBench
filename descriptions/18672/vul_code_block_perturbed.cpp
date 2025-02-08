if( EOLabelSize <= 0 || EOLabelSize > 100 * 1024 * 1024 )
        return false;
    if( VSIFSeekL( fp, nStartEOL, SEEK_SET ) != 0 )
    {
        CPLError(CE_Failure, CPLE_AppDefined, "Error seeking to EOL");
        return false;
    }
    char* pszChunkEOL = (char*) VSIMalloc(EOLabelSize+1);
    if( pszChunkEOL == nullptr )
        return false;
    nBytesRead = static_cast<int>(VSIFReadL( pszChunkEOL, 1, EOLabelSize, fp ));
    pszChunkEOL[nBytesRead] = '\0';
    osHeaderText += pszChunkEOL + nSkipEOLLBLSize;
    VSIFree(pszChunkEOL);
    CSLDestroy(papszKeywordList);
    papszKeywordList = nullptr;
    pszHeaderNext = osHeaderText.c_str();
    return Parse();