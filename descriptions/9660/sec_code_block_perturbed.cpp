GUInt32 nRawBytes = nRawTileWidth * nRawYSize * poGDS->nBands * poGDS->sHeader.nBitDepth / 8;

    //Direct read optimization
    if(poGDS->nBands == 1 && poGDS->sHeader.nBitDepth >= 8 &&
       nRawTileWidth == static_cast<GUInt32>(nBlockXSize) &&
       nRawYSize == static_cast<GUInt32>(nBlockYSize))
    {
        if(CE_None != poGDS->ReadTile(nBlockXOff, nBlockYOff,
                                      reinterpret_cast<GByte*>(pImage),
                                      nRawBytes, nRawTileWidth, nRawYSize))
        {
            CPLError(CE_Failure, CPLE_FileIO,
                     "Failed to read tile xOff %d yOff %d",
                     nBlockXOff, nBlockYOff);
            return CE_Failure;
        }
        return CE_None;
    }
#ifdef DEBUG
    CPLDebug("RMF", "IReadBlock nBand %d, RawSize [%d, %d], Bits %d",
             nBand, nRawTileWidth, nRawYSize, (int)poGDS->sHeader.nBitDepth);
#endif //DEBUG
    if(poGDS->pabyCurrentTile == nullptr ||
       poGDS->nCurrentTileXOff != nBlockXOff ||
       poGDS->nCurrentTileYOff != nBlockYOff ||
       poGDS->nCurrentTileBytes != nRawBytes)
    {
        if(poGDS->pabyCurrentTile == nullptr)
        {
            GUInt32 nMaxTileBytes = poGDS->sHeader.nTileWidth *
                                    poGDS->sHeader.nTileHeight *
                                    poGDS->nBands *
                                    poGDS->sHeader.nBitDepth / 8;
            poGDS->pabyCurrentTile =
               reinterpret_cast<GByte*>(VSIMalloc(std::max(1U, nMaxTileBytes)));
            if(!poGDS->pabyCurrentTile)
            {
                CPLError(CE_Failure, CPLE_OutOfMemory,
                         "Can't allocate tile block of size %lu.\n%s",
                         static_cast<unsigned long>(nMaxTileBytes),
                         VSIStrerror(errno));
                poGDS->nCurrentTileBytes = 0;
                return CE_Failure;
            }
        }

        poGDS->nCurrentTileXOff = nBlockXOff;
        poGDS->nCurrentTileYOff = nBlockYOff;
        poGDS->nCurrentTileBytes = nRawBytes;

        if(CE_None != poGDS->ReadTile(nBlockXOff, nBlockYOff,
                                      poGDS->pabyCurrentTile, nRawBytes,
                                      nRawTileWidth, nRawYSize))
        {
            CPLError(CE_Failure, CPLE_FileIO,
                     "Failed to read tile xOff %d yOff %d",
                     nBlockXOff, nBlockYOff);
            poGDS->nCurrentTileBytes = 0;
            return CE_Failure;
        }
    }