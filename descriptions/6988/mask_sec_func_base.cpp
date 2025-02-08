CPLErr VRTRawRasterBand::SetRawLink( const char *pszFilename,
                                     const char *pszVRTPath,
                                     int bRelativeToVRTIn,
                                     vsi_l_offset nImageOffset,
                                     int nPixelOffset, int nLineOffset,
                                     const char *pszByteOrder )

{
    ClearRawLink();

    reinterpret_cast<VRTDataset *>( poDS )->SetNeedsFlush();

/* -------------------------------------------------------------------- */
/*      Prepare filename.                                               */
/* -------------------------------------------------------------------- */
    if( pszFilename == nullptr )
    {
        CPLError( CE_Warning, CPLE_AppDefined,
                  "Missing <SourceFilename> element in VRTRasterBand." );
        return CE_Failure;
    }

    char *pszExpandedFilename = nullptr;
    if( pszVRTPath != nullptr && bRelativeToVRTIn )
    {
        pszExpandedFilename = CPLStrdup(
            CPLProjectRelativeFilename( pszVRTPath, pszFilename ) );
    }
    else
    {
        pszExpandedFilename = CPLStrdup( pszFilename );
    }

/* -------------------------------------------------------------------- */
/*      Try and open the file.  We always use the large file API.       */
/* -------------------------------------------------------------------- */
    CPLPushErrorHandler(CPLQuietErrorHandler);
    FILE *fp = CPLOpenShared( pszExpandedFilename, "rb+", TRUE );

    if( fp == nullptr )
        fp = CPLOpenShared( pszExpandedFilename, "rb", TRUE );

    if( fp == nullptr
        && reinterpret_cast<VRTDataset *>( poDS )->GetAccess() == GA_Update )
    {
        fp = CPLOpenShared( pszExpandedFilename, "wb+", TRUE );
    }
    CPLPopErrorHandler();
    CPLErrorReset();

    if( fp == nullptr )
    {
        CPLError( CE_Failure, CPLE_OpenFailed,
                  "Unable to open %s.%s",
                  pszExpandedFilename, VSIStrerror( errno ) );

        CPLFree( pszExpandedFilename );
        return CE_Failure;
    }

    CPLFree( pszExpandedFilename );

    // <MASK>

/* -------------------------------------------------------------------- */
/*      Create a corresponding RawRasterBand.                           */
/* -------------------------------------------------------------------- */
    m_poRawRaster = new RawRasterBand( fp, nImageOffset, nPixelOffset,
                                       nLineOffset, GetRasterDataType(),
                                       bNative, GetXSize(), GetYSize(), TRUE );

/* -------------------------------------------------------------------- */
/*      Reset block size to match the raw raster.                       */
/* -------------------------------------------------------------------- */
    m_poRawRaster->GetBlockSize( &nBlockXSize, &nBlockYSize );

    return CE_None;
}