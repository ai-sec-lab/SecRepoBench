static exr_result_t
fasthuf_initialize (
    const struct _internal_exr_context* pctxt,
    FastHufDecoder*                     fhd,
    const uint8_t**                     table,
    int                                 numBytes,
    int                                 minSymbol,
    int                                 maxSymbol,
    int                                 rleSymbol)
{
    fhd->_rleSymbol     = rleSymbol;
    fhd->_numSymbols    = 0;
    fhd->_minCodeLength = 255;
    fhd->_maxCodeLength = 0;

    //
    // The 'base' table is the minimum code at each code length. base[i]
    // is the smallest code (numerically) of length i.
    //

    uint64_t base[MAX_CODE_LEN + 1];

    //
    // The 'offset' table is the position (in sorted order) of the first id
    // of a given code length. Array is indexed by code length, like base.
    //

    uint64_t offset[MAX_CODE_LEN + 1];

    //
    // Count of how many codes at each length there are. Array is
    // indexed by code length, like base and offset.
    //

    size_t codeCount[MAX_CODE_LEN + 1];

    for (int i = 0; i <= MAX_CODE_LEN; ++i)
    {
        codeCount[i] = 0;
        base[i]      = 0xffffffffffffffffULL;
        offset[i]    = 0;
    }

    //
    // Count the number of codes, the min/max code lengths, the number of
    // codes with each length, and record symbols with non-zero code
    // length as we find them.
    //

    const uint8_t* currByte     = *table;
    uint64_t       currBits     = 0;
    int            currBitCount = 0;

    const uint8_t* topByte = *table + numBytes;

    for (uint64_t symbol = (uint64_t) minSymbol; symbol <= (uint64_t) maxSymbol;
         symbol++)
    {
        if (currByte >= topByte)
        {
            if (pctxt)
                pctxt->print_error (
                    pctxt,
                    EXR_ERR_CORRUPT_CHUNK,
                    "Error decoding Huffman table (Truncated table data).");
            return EXR_ERR_CORRUPT_CHUNK;
        }

        //
        // Next code length - either:
        //       0-58  (literal code length)
        //       59-62 (various lengths runs of 0)
        //       63    (run of n 0's, with n is the next 8 bits)
        //

        uint64_t codeLength =
            fasthuf_read_bits (6, &currBits, &currBitCount, &currByte);

        // <MASK>

        if (symbol > (uint64_t) maxSymbol)
        {
            if (pctxt)
                pctxt->print_error (
                    pctxt,
                    EXR_ERR_CORRUPT_CHUNK,
                    "Error decoding Huffman table (Run beyond end of table).");
            return EXR_ERR_CORRUPT_CHUNK;
        }
    }

    for (int i = 0; i < MAX_CODE_LEN; ++i)
        fhd->_numSymbols += codeCount[i];

    if ((size_t) fhd->_numSymbols > sizeof (fhd->_idToSymbol) / sizeof (int))
    {
        if (pctxt)
            pctxt->print_error (
                pctxt,
                EXR_ERR_CORRUPT_CHUNK,
                "Error decoding Huffman table (Too many symbols).");
        return EXR_ERR_CORRUPT_CHUNK;
    }

    //
    // Compute base - once we have the code length counts, there
    //                is a closed form solution for this
    //

    {
        double* countTmp = (double*) offset; /* temp space */

        for (int l = fhd->_minCodeLength; l <= fhd->_maxCodeLength; ++l)
        {
            countTmp[l] = (double) codeCount[l] *
                          (double) (2ll << (fhd->_maxCodeLength - l));
        }

        for (int l = fhd->_minCodeLength; l <= fhd->_maxCodeLength; ++l)
        {
            double tmp = 0;

            for (int k = l + 1; k <= fhd->_maxCodeLength; ++k)
                tmp += countTmp[k];

            tmp /= (double) (2ll << (fhd->_maxCodeLength - l));

            base[l] = (uint64_t) ceil (tmp);
        }
    }

    //
    // Compute offset - these are the positions of the first
    //                  id (not symbol) that has length [i]
    //

    offset[fhd->_maxCodeLength] = 0;

    for (int i = fhd->_maxCodeLength - 1; i >= fhd->_minCodeLength; i--)
        offset[i] = offset[i + 1] + codeCount[i + 1];

    //
    // Allocate and fill the symbol-to-id mapping. Smaller Ids should be
    // mapped to less-frequent symbols (which have longer codes). Use
    // the offset table to tell us where the id's for a given code
    // length start off.
    //

    uint64_t mapping[MAX_CODE_LEN + 1];
    for (int i = 0; i < MAX_CODE_LEN + 1; ++i)
        mapping[i] = -1;
    for (int i = fhd->_minCodeLength; i <= fhd->_maxCodeLength; ++i)
        mapping[i] = offset[i];

    currByte     = *table;
    currBits     = 0;
    currBitCount = 0;

    //
    // Although we could have created an uncompressed list of symbols in our
    // decoding loop above, it's faster to decode the compressed data again
    //
    for (uint64_t symbol = (uint64_t) minSymbol; symbol <= (uint64_t) maxSymbol;
         symbol++)
    {
        uint64_t codeLength =
            fasthuf_read_bits (6, &currBits, &currBitCount, &currByte);

        if (codeLength < (uint64_t) SHORT_ZEROCODE_RUN)
        {
            if (codeLength == 0) continue;

            if (mapping[codeLength] >= (uint64_t) fhd->_numSymbols)
            {
                if (pctxt)
                    pctxt->print_error (
                        pctxt,
                        EXR_ERR_CORRUPT_CHUNK,
                        "Huffman decode error (Invalid symbol in header)");
                return EXR_ERR_CORRUPT_CHUNK;
            }
            fhd->_idToSymbol[mapping[codeLength]] = symbol;
            mapping[codeLength]++;
        }
        else if (codeLength == (uint64_t) LONG_ZEROCODE_RUN)
            symbol +=
                fasthuf_read_bits (8, &currBits, &currBitCount, &currByte) +
                SHORTEST_LONG_RUN - 1;
        else
            symbol += codeLength - SHORT_ZEROCODE_RUN + 1;
    }

    *table = currByte;

    return FastHufDecoder_buildTables (pctxt, fhd, base, offset);
}