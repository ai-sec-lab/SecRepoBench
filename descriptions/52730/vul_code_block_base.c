    symbol +=
                fasthuf_read_bits (8, &currBits, &currBitCount, &currByte) +
                SHORTEST_LONG_RUN - 1;
        else
            symbol += codeLen - SHORT_ZEROCODE_RUN + 1;