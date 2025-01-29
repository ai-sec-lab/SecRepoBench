*copied = 0;

    while (1) {
        uint64_t size;
        uint32_t tag;
        int head_size = 8;

        if (b - input_buffer >= input_len - 16)
            break;

        size = AV_RB32(b);
        b += 4;
        if (size == 1) {
            size = AV_RB64(b);
            b += 8;
            head_size = 16;
        }
        /* invalid ISOBMFF size */
        if (size > 0 && size <= head_size)
            return AVERROR_INVALIDDATA;
        if (size > 0)
            size -= head_size;

        tag = AV_RL32(b);
        b += 4;
        if (tag == MKTAG('j', 'x', 'l', 'p')) {
            b += 4;
            size -= 4;
        }

        if (tag == MKTAG('j', 'x', 'l', 'c') || tag == MKTAG('j', 'x', 'l', 'p')) {
            /*
             * size = 0 means "until EOF". this is legal but uncommon
             * here we just set it to the remaining size of the probe buffer
             * which at this point should always be nonnegative
             */
            if (size == 0 || size > input_len - (b - input_buffer))
                size = input_len - (b - input_buffer);

            if (size > buflen - *copied)
                size = buflen - *copied;
            /*
             * arbitrary chunking of the payload makes this memcpy hard to avoid
             * in practice this will only be performed one or two times at most
             */
            memcpy(buffer + *copied, b, size);
            *copied += size;
        }
        b += size;
        if (b >= input_buffer + input_len || *copied >= buflen)
            break;
    }

    return b - input_buffer;