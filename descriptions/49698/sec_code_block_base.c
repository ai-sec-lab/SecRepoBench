const uint64_t hufInfoBlockSize           = 5 * sizeof (uint32_t);

    if (decode) pctxt = EXR_CCTXT (decode->context);
    //
    // need at least 20 bytes for header
    //
    if (nCompressed < 20)
    {
        if (nRaw != 0) return EXR_ERR_INVALID_ARGUMENT;
        return EXR_ERR_SUCCESS;
    }

    if (sparebytes != internal_exr_huf_decompress_spare_bytes ())
        return EXR_ERR_INVALID_ARGUMENT;

    im = readUInt (compressed);
    iM = readUInt (compressed + 4);
    // uint32_t tableLength = readUInt (compressed + 8);
    nBits = readUInt (compressed + 12);
    // uint32_t future = readUInt (compressed + 16);

    if (im >= HUF_ENCSIZE || iM >= HUF_ENCSIZE) return EXR_ERR_CORRUPT_CHUNK;

    ptr = compressed + hufInfoBlockSize;

    nBytes = (((uint64_t) (nBits) + 7)) / 8;

    // must be nBytes remaining in buffer
    if (hufInfoBlockSize + nBytes > nCompressed) return EXR_ERR_OUT_OF_MEMORY;

    //
    // Fast decoder needs at least 2x64-bits of compressed data, and
    // needs to be run-able on this platform. Otherwise, fall back
    // to the original decoder
    //
    if (fasthuf_decode_enabled () && nBits > 128)
    {
        FastHufDecoder* fhd = (FastHufDecoder*) spare;

        rv = fasthuf_initialize (pctxt, fhd, &ptr, nCompressed - hufInfoBlockSize, im, iM, iM);
        if (rv == EXR_ERR_SUCCESS)
        {
            if ( (uint64_t)(ptr - compressed) + nBytes > nCompressed )
                return EXR_ERR_OUT_OF_MEMORY;
            rv = fasthuf_decode (pctxt, fhd, ptr, nBits, raw, nRaw);
        }
    }
    else
    {
        uint64_t* freq     = (uint64_t*) spare;
        HufDec*   hdec     = (HufDec*) (freq + HUF_ENCSIZE);
        uint64_t  nLeft    = nCompressed - 20;
        uint64_t  nTableSz = 0;

        hufClearDecTable (hdec);
        hufUnpackEncTable (&ptr, &nLeft, im, iM, freq);

        if (nBits > 8 * nLeft) return EXR_ERR_CORRUPT_CHUNK;

        rv = hufBuildDecTable (freq, im, iM, hdec);
        if (rv == EXR_ERR_SUCCESS)
            rv = hufDecode (freq, hdec, ptr, nBits, iM, nRaw, raw);

        hufFreeDecTable (hdec);
    }