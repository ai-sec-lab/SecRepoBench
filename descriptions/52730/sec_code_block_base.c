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