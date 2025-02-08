{
        ofpbuf_put_uninit(b, n); /* b->size gets increased. */
        memmove((char *) b->data + offset + n, (char *) b->data + offset,
                b->size - offset - n);
        memcpy((char *) b->data + offset, bufferdata, n);
    } else {
        ovs_assert(offset == b->size);
        ofpbuf_put(b, bufferdata, n);
    }