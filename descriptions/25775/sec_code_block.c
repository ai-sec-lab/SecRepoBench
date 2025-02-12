if (size < 8 || p->buf_size - i < size)
            return 0;
        i += 8;