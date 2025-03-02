if (codeLen < (uint64_t) SHORT_ZEROCODE_RUN)
        {
            if (codeLen == 0) continue;

            if (codeLen < fhd->_minCodeLength) fhd->_minCodeLength = codeLen;

            if (codeLen > fhd->_maxCodeLength) fhd->_maxCodeLength = codeLen;

            codeCount[codeLen]++;
        }
        else if (codeLen == (uint64_t) LONG_ZEROCODE_RUN)
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

            symbol +=
                fasthuf_read_bits (8, &currBits, &currBitCount, &currByte) +
                SHORTEST_LONG_RUN - 1;
        }
        else
            symbol += codeLen - SHORT_ZEROCODE_RUN + 1;