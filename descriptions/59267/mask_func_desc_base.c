exr_result_t
DwaCompressor_uncompress (
    DwaCompressor* me,
    const uint8_t* inPtr,
    uint64_t       iSize,
    void*          uncompressed_data,
    uint64_t       uncompressed_size)
{
    uint64_t     headerSize = NUM_SIZES_SINGLE * sizeof (uint64_t);
    exr_result_t rv         = EXR_ERR_SUCCESS;
    uint64_t     counters[NUM_SIZES_SINGLE];
    uint64_t     version;
    uint64_t     unknownUncompressedSize;
    uint64_t     unknownCompressedSize;
    uint64_t     acCompressedSize;
    uint64_t     dcCompressedSize;
    uint64_t     rleCompressedSize;
    uint64_t     rleUncompressedSize;
    uint64_t     rleRawSize;

    uint64_t totalAcUncompressedCount;
    uint64_t totalDcUncompressedCount;

    uint64_t acCompression;

    size_t         outBufferSize;
    uint64_t       compressedSize;
    const uint8_t* dataPtr;
    uint64_t       dataLeft;
    uint8_t*       outBufferEnd;
    uint8_t*       packedAcBufferEnd;
    uint8_t*       packedDcBufferEnd;
    // Define pointers for compressed data sections: UNKNOWN, AC, DC, and RLE.
    // Verify that the input size is adequate for the expected header size.
    // Initialize the uncompressed data buffer to zero.
    // Copy header information into an array and convert it from XDR format to native format.
    // Extract various sizes and counters related to compression from the header information.
    // Calculate the total size of compressed data sections.
    // Adjust the data pointer to skip the header and calculate the remaining data size.
    // Verify that compressed data sizes do not exceed the input size to prevent overflow.
    // Check version information to determine channel rules, updating them if necessary.
    // Initialize buffers and allocate the output buffer for uncompressed data.
    // Determine buffer pointers for AC and DC data, if available.
    // Set pointers to the start of each compressed data section: UNKNOWN, AC, DC, and RLE.
    // <MASK>

    //
    // Sanity check that the version is something we expect. Right now,
    // we can decode version 0, 1, and 2. v1 adds 'end of block' symbols
    // to the AC RLE. v2 adds channel classification rules at the
    // start of the data block.
    //

    if (version > 2) { return EXR_ERR_BAD_CHUNK_LEADER; }

    rv = DwaCompressor_setupChannelData (me);

    //
    // Uncompress the UNKNOWN data into _planarUncBuffer[UNKNOWN]
    //

    if (unknownCompressedSize > 0)
    {
        if (unknownUncompressedSize > me->_planarUncBufferSize[UNKNOWN])
        {
            return EXR_ERR_CORRUPT_CHUNK;
        }

        if (EXR_ERR_SUCCESS != exr_uncompress_buffer (
                                   me->_decode->context,
                                   compressedUnknownBuf,
                                   unknownCompressedSize,
                                   me->_planarUncBuffer[UNKNOWN],
                                   unknownUncompressedSize,
                                   NULL))
        {
            return EXR_ERR_CORRUPT_CHUNK;
        }
    }

    //
    // Uncompress the AC data into _packedAcBuffer
    //

    if (acCompressedSize > 0)
    {
        if (!me->_packedAcBuffer ||
            totalAcUncompressedCount * sizeof (uint16_t) >
                me->_packedAcBufferSize)
        {
            return EXR_ERR_CORRUPT_CHUNK;
        }

        //
        // Don't trust the user to get it right, look in the file.
        //

        switch (acCompression)
        {
            case STATIC_HUFFMAN:
                rv = internal_huf_decompress (
                    me->_decode,
                    compressedAcBuf,
                    acCompressedSize,
                    (uint16_t*) me->_packedAcBuffer,
                    totalAcUncompressedCount,
                    me->_decode->scratch_buffer_1,
                    me->_decode->scratch_alloc_size_1);
                if (rv != EXR_ERR_SUCCESS) { return rv; }
                break;

            case DEFLATE: {
                size_t destLen;

                rv = exr_uncompress_buffer (
                    me->_decode->context,
                    compressedAcBuf,
                    acCompressedSize,
                    me->_packedAcBuffer,
                    totalAcUncompressedCount * sizeof (uint16_t),
                    &destLen);
                if (rv != EXR_ERR_SUCCESS) return rv;

                if (totalAcUncompressedCount * sizeof (uint16_t) != destLen)
                {
                    return EXR_ERR_CORRUPT_CHUNK;
                }
            }
            break;

            default: return EXR_ERR_CORRUPT_CHUNK; break;
        }
    }

    //
    // Uncompress the DC data into _packedDcBuffer
    //

    if (dcCompressedSize > 0)
    {
        size_t destLen;
        size_t uncompBytes = totalDcUncompressedCount * sizeof (uint16_t);
        if (uncompBytes > me->_packedDcBufferSize)
        {
            return EXR_ERR_CORRUPT_CHUNK;
        }

        rv = internal_decode_alloc_buffer (
            me->_decode,
            EXR_TRANSCODE_BUFFER_SCRATCH1,
            &(me->_decode->scratch_buffer_1),
            &(me->_decode->scratch_alloc_size_1),
            uncompBytes);

        if (rv != EXR_ERR_SUCCESS) return rv;

        rv = exr_uncompress_buffer (
            me->_decode->context,
            compressedDcBuf,
            dcCompressedSize,
            me->_decode->scratch_buffer_1,
            uncompBytes,
            &destLen);
        if (rv != EXR_ERR_SUCCESS || (uncompBytes != destLen))
        {
            return EXR_ERR_CORRUPT_CHUNK;
        }

        internal_zip_reconstruct_bytes (
            me->_packedDcBuffer, me->_decode->scratch_buffer_1, uncompBytes);
    }
    else
    {
        // if the compressed size is 0, then the uncompressed size must also be zero
        if (totalDcUncompressedCount != 0) { return EXR_ERR_CORRUPT_CHUNK; }
    }

    //
    // Uncompress the RLE data into _rleBuffer, then unRLE the results
    // into _planarUncBuffer[RLE]
    //

    if (rleRawSize > 0)
    {
        size_t dstLen;

        if (rleUncompressedSize > me->_rleBufferSize ||
            rleRawSize > me->_planarUncBufferSize[RLE])
        {
            return EXR_ERR_CORRUPT_CHUNK;
        }

        if (EXR_ERR_SUCCESS != exr_uncompress_buffer (
                                   me->_decode->context,
                                   compressedRleBuf,
                                   rleCompressedSize,
                                   me->_rleBuffer,
                                   rleUncompressedSize,
                                   &dstLen))
        {
            return EXR_ERR_CORRUPT_CHUNK;
        }

        if (dstLen != rleUncompressedSize) { return EXR_ERR_CORRUPT_CHUNK; }

        if (internal_rle_decompress (
                me->_planarUncBuffer[RLE],
                rleRawSize,
                (const uint8_t*) me->_rleBuffer,
                rleUncompressedSize) != rleRawSize)
        {
            return EXR_ERR_CORRUPT_CHUNK;
        }
    }

    //
    // Determine the start of each row in the output buffer
    //
    for (int c = 0; c < me->_numChannels; ++c)
    {
        me->_channelData[c].processed = 0;
    }

    for (int y = me->_min[1]; y <= me->_max[1]; ++y)
    {
        for (int c = 0; c < me->_numChannels; ++c)
        {
            ChannelData*               cd   = &(me->_channelData[c]);
            exr_coding_channel_info_t* chan = cd->chan;

            if ((y % chan->y_samples) != 0) continue;

            rv = DctCoderChannelData_push_row (
                me->alloc_fn, me->free_fn, &(cd->_dctData), outBufferEnd);
            if (rv != EXR_ERR_SUCCESS) return rv;

            outBufferEnd += chan->width * chan->bytes_per_element;
        }
    }

    //
    // Setup to decode each block of 3 channels that need to
    // be handled together
    //

    for (int csc = 0; csc < me->_numCscChannelSets; ++csc)
    {
        LossyDctDecoder decoder;
        CscChannelSet*  cset = &(me->_cscChannelSets[csc]);

        int rChan = cset->idx[0];
        int gChan = cset->idx[1];
        int bChan = cset->idx[2];

        if (me->_channelData[rChan].compression != LOSSY_DCT ||
            me->_channelData[gChan].compression != LOSSY_DCT ||
            me->_channelData[bChan].compression != LOSSY_DCT)
        {
            return EXR_ERR_CORRUPT_CHUNK;
        }

        rv = LossyDctDecoderCsc_construct (
            &decoder,
            &(me->_channelData[rChan]._dctData),
            &(me->_channelData[gChan]._dctData),
            &(me->_channelData[bChan]._dctData),
            packedAcBufferEnd,
            packedAcBufferEnd + totalAcUncompressedCount * sizeof (uint16_t),
            packedDcBufferEnd,
            dwaCompressorToLinear,
            me->_channelData[rChan].chan->width,
            me->_channelData[rChan].chan->height);

        if (rv == EXR_ERR_SUCCESS)
            rv = LossyDctDecoder_execute (me->alloc_fn, me->free_fn, &decoder);

        packedAcBufferEnd += decoder._packedAcCount * sizeof (uint16_t);

        packedDcBufferEnd += decoder._packedDcCount * sizeof (uint16_t);

        me->_channelData[rChan].processed = 1;
        me->_channelData[gChan].processed = 1;
        me->_channelData[bChan].processed = 1;

        if (rv != EXR_ERR_SUCCESS) { return rv; }
    }

    //
    // Setup to handle the remaining channels by themselves
    //

    for (int c = 0; c < me->_numChannels; ++c)
    {
        ChannelData*               cd        = &(me->_channelData[c]);
        exr_coding_channel_info_t* chan      = cd->chan;
        DctCoderChannelData*       dcddata   = &(cd->_dctData);
        int                        pixelSize = chan->bytes_per_element;

        if (cd->processed) continue;

        switch (cd->compression)
        {
            case LOSSY_DCT:

                //
                // Setup a single-channel lossy DCT decoder pointing
                // at the output buffer
                //

                {
                    const uint16_t* linearLut = NULL;
                    LossyDctDecoder decoder;

                    if (!chan->p_linear) linearLut = dwaCompressorToLinear;

                    rv = LossyDctDecoder_construct (
                        &decoder,
                        dcddata,
                        packedAcBufferEnd,
                        packedAcBufferEnd +
                            totalAcUncompressedCount * sizeof (uint16_t),
                        packedDcBufferEnd,
                        linearLut,
                        chan->width,
                        chan->height);

                    if (rv == EXR_ERR_SUCCESS)
                        rv = LossyDctDecoder_execute (
                            me->alloc_fn, me->free_fn, &decoder);

                    packedAcBufferEnd +=
                        (size_t) decoder._packedAcCount * sizeof (uint16_t);

                    packedDcBufferEnd +=
                        (size_t) decoder._packedDcCount * sizeof (uint16_t);

                    if (rv != EXR_ERR_SUCCESS) { return rv; }
                }

                break;

            case RLE:

                //
                // For the RLE case, the data has been un-RLE'd into
                // planarUncRleEnd[], but is still split out by bytes.
                // We need to rearrange the bytes back into the correct
                // order in the output buffer;
                //

                {
                    int row = 0;

                    for (int y = me->_min[1]; y <= me->_max[1]; ++y)
                    {
                        uint8_t* dst;
                        if ((y % chan->y_samples) != 0) continue;

                        dst = dcddata->_rows[row];

                        if (pixelSize == 2)
                        {
                            interleaveByte2 (
                                dst,
                                cd->planarUncRleEnd[0],
                                cd->planarUncRleEnd[1],
                                chan->width);

                            cd->planarUncRleEnd[0] += chan->width;
                            cd->planarUncRleEnd[1] += chan->width;
                        }
                        else
                        {
                            for (int x = 0; x < chan->width; ++x)
                            {
                                for (int byte = 0; byte < pixelSize; ++byte)
                                {
                                    *dst++ = *cd->planarUncRleEnd[byte]++;
                                }
                            }
                        }

                        row++;
                    }
                }

                break;

            case UNKNOWN:

                //
                // In the UNKNOWN case, data is already in planarUncBufferEnd
                // and just needs to copied over to the output buffer
                //

                {
                    int    row = 0;
                    size_t dstScanlineSize =
                        (size_t) chan->width * (size_t) pixelSize;

                    for (int y = me->_min[1]; y <= me->_max[1]; ++y)
                    {
                        if ((y % chan->y_samples) != 0) continue;

                        //
                        // sanity check for buffer data lying within range
                        //
                        if ((cd->planarUncBufferEnd +
                             (size_t) (dstScanlineSize)) >
                            (me->_planarUncBuffer[UNKNOWN] +
                             me->_planarUncBufferSize[UNKNOWN]))
                        {
                            return EXR_ERR_CORRUPT_CHUNK;
                        }

                        memcpy (
                            dcddata->_rows[row],
                            cd->planarUncBufferEnd,
                            dstScanlineSize);

                        cd->planarUncBufferEnd += dstScanlineSize;
                        row++;
                    }
                }

                break;

            case NUM_COMPRESSOR_SCHEMES:
            default: return EXR_ERR_CORRUPT_CHUNK; break;
        }

        cd->processed = 1;
    }

    return rv;
}