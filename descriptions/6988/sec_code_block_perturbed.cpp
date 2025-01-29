if( !RAWDatasetCheckMemoryUsage(
                        nRasterXSize, nRasterYSize, 1,
                        nPixelOffset, lineOffset, nImageOffset, 0,
                        reinterpret_cast<VSILFILE*>(fp)) )
    {
        CPLCloseShared(fp);
        return CE_Failure;
    }

    m_pszSourceFilename = CPLStrdup(pszFilename);
    m_bRelativeToVRT = bRelativeToVRTIn;

/* -------------------------------------------------------------------- */
/*      Work out if we are in native mode or not.                       */
/* -------------------------------------------------------------------- */
    bool bNative = true;

    if( pszByteOrder != nullptr )
    {
        if( EQUAL(pszByteOrder,"LSB") )
            bNative = CPL_TO_BOOL(CPL_IS_LSB);
        else if( EQUAL(pszByteOrder,"MSB") )
            bNative = !CPL_IS_LSB;
        else
        {
            CPLError( CE_Failure, CPLE_AppDefined,
                      "Illegal ByteOrder value '%s', should be LSB or MSB.",
                      pszByteOrder );
            CPLCloseShared(fp);
            return CE_Failure;
        }
    }