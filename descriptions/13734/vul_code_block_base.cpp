GDALDataset *poSrcDS = nullptr;
    if( nRasterXSize == 0 || nRasterYSize == 0 ||
        eDataType == static_cast<GDALDataType>(-1) ||
        nBlockXSize == 0 || nBlockYSize == 0 )
    {
        /* ----------------------------------------------------------------- */
        /*      Open the file (shared).                                      */
        /* ----------------------------------------------------------------- */
        const int nOpenFlags = GDAL_OF_RASTER | GDAL_OF_VERBOSE_ERROR;
        if( bShared )
        {
            // We no longer use GDAL_OF_SHARED as this can cause quite
            // annoying reference cycles in situations like you have
            // foo.tif and foo.tif.ovr, the later being actually a VRT file
            // that points to foo.tif
            auto oIter = oMapSharedSources.find(pszSrcDSName);
            if( oIter != oMapSharedSources.end() )
            {
                poSrcDS = oIter->second;
                poSrcDS->Reference();
            }
            else
            {
                poSrcDS = static_cast<GDALDataset *>( GDALOpenEx(
                        pszSrcDSName, nOpenFlags, nullptr,
                        (const char* const* )papszOpenOptions, nullptr ) );
                if( poSrcDS )
                {
                    oMapSharedSources[pszSrcDSName] = poSrcDS;
                }
            }
        }
        else
        {
            poSrcDS = static_cast<GDALDataset *>( GDALOpenEx(
                        pszSrcDSName, nOpenFlags, nullptr,
                        (const char* const* )papszOpenOptions, nullptr ) );
        }
    }
    else
    {
        /* ----------------------------------------------------------------- */
        /*      Create a proxy dataset                                       */
        /* ----------------------------------------------------------------- */
        CPLString osUniqueHandle( CPLSPrintf("%p", pUniqueHandle) );
        GDALProxyPoolDataset * const proxyDS =
            new GDALProxyPoolDataset( pszSrcDSName, nRasterXSize, nRasterYSize,
                                      GA_ReadOnly, bShared, nullptr, nullptr,
                                      osUniqueHandle.c_str() );
        proxyDS->SetOpenOptions(papszOpenOptions);
        poSrcDS = proxyDS;

        // Only the information of rasterBand nSrcBand will be accurate
        // but that's OK since we only use that band afterwards.
        //
        // Previously this added a src band for every band <= nSrcBand, but this becomes
        // prohibitely expensive for files with a large number of bands. This optimization
        // only adds the desired band and the rest of the bands will simply be initialized with a nullptr.
        // This assumes no other code here accesses any of the lower bands in the GDALProxyPoolDataset.
        // It has been suggested that in addition, we should to try share GDALProxyPoolDataset between multiple
        // Simple Sources, which would save on memory for papoBands. For now, that's not implemented.
        proxyDS->AddSrcBand(nSrcBand, eDataType, nBlockXSize, nBlockYSize);

        if( bGetMaskBand )
        {
          GDALProxyPoolRasterBand *poMaskBand =
              dynamic_cast<GDALProxyPoolRasterBand *>(
              proxyDS->GetRasterBand(nSrcBand) );
          if( poMaskBand == nullptr )
          {
              CPLError(
                  CE_Fatal, CPLE_AssertionFailed, "dynamic_cast failed." );
          }
          else
          {
              poMaskBand->AddSrcMaskBandDescription(
                  eDataType, nBlockXSize, nBlockYSize );
          }
        }
    }

    CSLDestroy(papszOpenOptions);

    CPLFree( pszSrcDSName );

    if( poSrcDS == nullptr )
        return CE_Failure;

/* -------------------------------------------------------------------- */
/*      Get the raster band.                                            */
/* -------------------------------------------------------------------- */

    m_poRasterBand = poSrcDS->GetRasterBand(nSrcBand);
    if( m_poRasterBand == nullptr )
    {
        poSrcDS->ReleaseRef();
        return CE_Failure;
    }