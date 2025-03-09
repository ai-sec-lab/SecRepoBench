if (length < 8 || p->buf_size - i < length)
            return 0;
        i += 8;