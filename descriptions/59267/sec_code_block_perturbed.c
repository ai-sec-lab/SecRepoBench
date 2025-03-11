const uint8_t* dataPtrEnd;
    const uint8_t* compressedUnknownBuf;
    const uint8_t* compressedAcBuf;
    const uint8_t* compressedDcBuf;
    const uint8_t* compressedRleBuf;

    if (iSize < headerSize) return EXR_ERR_CORRUPT_CHUNK;

    //
    // Flip the counters from XDR to NATIVE
    //

    memset (uncompressed_data, 0, uncompressed_size);

    memcpy (counters, inPtr, headerSize);
    priv_to_native64 (counters, NUM_SIZES_SINGLE);

    //
    // Unwind all the counter info
    //
    version                 = counters[VERSION];
    unknownUncompressedSize = counters[UNKNOWN_UNCOMPRESSED_SIZE];
    unknownCompressedSize   = counters[UNKNOWN_COMPRESSED_SIZE];
    acCompressedSize        = counters[AC_COMPRESSED_SIZE];
    dcCompressedBytes        = counters[DC_COMPRESSED_SIZE];
    rleCompressedSize       = counters[RLE_COMPRESSED_SIZE];
    rleUncompressedSize     = counters[RLE_UNCOMPRESSED_SIZE];
    rleRawSize              = counters[RLE_RAW_SIZE];

    totalAcUncompressedCount = counters[AC_UNCOMPRESSED_COUNT];
    totalDcUncompressedCount = counters[DC_UNCOMPRESSED_COUNT];

    acCompression = counters[AC_COMPRESSION];

    compressedSize = unknownCompressedSize + acCompressedSize +
                     dcCompressedBytes + rleCompressedSize;

    dataPtrEnd = inPtr + iSize;
    dataPtr  = inPtr + headerSize;
    dataLeft = iSize - headerSize;

    /* Both the sum and individual sizes are checked in case of overflow. */
    if (iSize < (headerSize + compressedSize) ||
        iSize < unknownCompressedSize || iSize < acCompressedSize ||
        iSize < dcCompressedBytes || iSize < rleCompressedSize)
    {
        return EXR_ERR_CORRUPT_CHUNK;
    }

    if ((int64_t) unknownUncompressedSize < 0 ||
        (int64_t) unknownCompressedSize < 0 || (int64_t) acCompressedSize < 0 ||
        (int64_t) dcCompressedBytes < 0 || (int64_t) rleCompressedSize < 0 ||
        (int64_t) rleUncompressedSize < 0 || (int64_t) rleRawSize < 0 ||
        (int64_t) totalAcUncompressedCount < 0 ||
        (int64_t) totalDcUncompressedCount < 0)
    {
        return EXR_ERR_CORRUPT_CHUNK;
    }

    if (version < 2)
    {
        me->_channelRules = sLegacyChannelRules;
        me->_channelRuleCount =
            sizeof (sLegacyChannelRules) / sizeof (Classifier);
    }
    else
    {
        uint64_t ruleSize;
        rv =
            DwaCompressor_readChannelRules (me, &dataPtr, &dataLeft, &ruleSize);

        headerSize += ruleSize;
    }

    if (rv != EXR_ERR_SUCCESS) return rv;

    outBufferSize = 0;
    rv            = DwaCompressor_initializeBuffers (me, &outBufferSize);
    if (rv != EXR_ERR_SUCCESS) return rv;

    //
    // Allocate _outBuffer, if we haven't done so already
    //

    // the C++ classes used to have one buffer size for compress / uncompress
    // but here we want to do zero-ish copy...
    outBufferEnd  = me->_decode->unpacked_buffer;
    outBufferSize = me->_decode->unpacked_alloc_size;

    //
    // Find the start of the RLE packed AC components and
    // the DC components for each channel. This will be handy
    // if you want to decode the channels in parallel later on.
    //

    packedAcBufferEnd = NULL;

    if (me->_packedAcBuffer) packedAcBufferEnd = me->_packedAcBuffer;

    packedDcBufferEnd = NULL;

    if (me->_packedDcBuffer) packedDcBufferEnd = me->_packedDcBuffer;

    //
    // UNKNOWN data is packed first, followed by the
    // Huffman-compressed AC, then the DC values,
    // and then the zlib compressed RLE data.
    //

    compressedUnknownBuf = dataPtr;

    compressedAcBuf =
        compressedUnknownBuf + (ptrdiff_t) (unknownCompressedSize);
    compressedDcBuf  = compressedAcBuf + (ptrdiff_t) (acCompressedSize);
    compressedRleBuf = compressedDcBuf + (ptrdiff_t) (dcCompressedBytes);

    if (compressedUnknownBuf >= dataPtrEnd ||
        dataPtr > compressedAcBuf ||
        compressedAcBuf >= dataPtrEnd ||
        dataPtr > compressedDcBuf ||
        compressedDcBuf >= dataPtrEnd ||
        dataPtr > compressedRleBuf ||
        compressedRleBuf >= dataPtrEnd ||
        (compressedRleBuf + rleCompressedSize) > dataPtrEnd)
    {
        return EXR_ERR_CORRUPT_CHUNK;
    }