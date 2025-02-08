CPLErr RIKRasterBand::IReadBlock( int nBlockXOff, int nBlockYOff,
                                  void * pImage )

{
    RIKDataset *poRDS = reinterpret_cast<RIKDataset *>( poDS );

    const GUInt32 blocks = poRDS->nHorBlocks * poRDS->nVertBlocks;
    const GUInt32 nBlockIndex = nBlockXOff + nBlockYOff * poRDS->nHorBlocks;
    const GUInt32 nBlockOffset = poRDS->pOffsets[nBlockIndex];

    GUInt32 nBlockSize = poRDS->nFileSize;
    for( GUInt32 bi = nBlockIndex + 1; bi < blocks; bi++ )
    {
        if( poRDS->pOffsets[bi] )
        {
            nBlockSize = poRDS->pOffsets[bi];
            break;
        }
    }
    nBlockSize -= nBlockOffset;

    GUInt32 pixels;

    pixels = poRDS->nBlockXSize * poRDS->nBlockYSize;

    if( !nBlockOffset || !nBlockSize
#ifdef RIK_SINGLE_BLOCK
        || nBlockIndex != RIK_SINGLE_BLOCK
#endif
        )
    {
        for( GUInt32 i = 0; i < pixels; i++ )
            reinterpret_cast<GByte *>( pImage )[i] = 0;
        return CE_None;
    }

    VSIFSeekL( poRDS->fp, nBlockOffset, SEEK_SET );

/* -------------------------------------------------------------------- */
/*      Read uncompressed block.                                        */
/* -------------------------------------------------------------------- */

    if( poRDS->options == 0x00 || poRDS->options == 0x40 )
    {
        VSIFReadL( pImage, 1, nBlockXSize * nBlockYSize, poRDS->fp );
        return CE_None;
    }

    // Read block to memory
    GByte *blockData = reinterpret_cast<GByte *>( VSI_MALLOC_VERBOSE(nBlockSize) );
    if( blockData == nullptr )
        return CE_Failure;
    if( VSIFReadL( blockData, 1, nBlockSize, poRDS->fp ) != nBlockSize )
    {
        VSIFree(blockData);
        return CE_Failure;
    }

/* -------------------------------------------------------------------- */
/*      Read RLE block.                                                 */
/* -------------------------------------------------------------------- */
    GUInt32 filePos = 0;
    GUInt32 imageIndex = 0;

    if( poRDS->options == 0x01 ||
        poRDS->options == 0x41 )
    {
        while( filePos+1 < nBlockSize && imageIndex < pixels )
        {
            GByte count = blockData[filePos++];
            GByte color = blockData[filePos++];

            for (GByte i = 0; imageIndex < pixels && i <= count; i++)
            {
                reinterpret_cast<GByte *>( pImage )[imageIndex++] = color;
            }
        }
    }

/* -------------------------------------------------------------------- */
/*      Read LZW block.                                                 */
/* -------------------------------------------------------------------- */

    else if( poRDS->options == 0x0b )
    {
        // <MASK>
    }

/* -------------------------------------------------------------------- */
/*      Read ZLIB block.                                                */
/* -------------------------------------------------------------------- */

    else if( poRDS->options == 0x0d )
    {
        uLong destLen = pixels;
        Byte *upsideDown = static_cast<Byte *>( CPLMalloc( pixels ) );

        uncompress( upsideDown, &destLen, blockData, nBlockSize );

        for (GUInt32 i = 0; i < poRDS->nBlockYSize; i++)
        {
          memcpy( reinterpret_cast<Byte *>( pImage ) + poRDS->nBlockXSize * i,
                    upsideDown + poRDS->nBlockXSize *
                                 (poRDS->nBlockYSize - i - 1),
                    poRDS->nBlockXSize );
        }

        CPLFree( upsideDown );
    }

    CPLFree( blockData );

    return CE_None;
}