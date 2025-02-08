{
        ofpbuf_put_uninit(b, n);
        memmove((char *) b->data + offset + n, (char *) b->data + offset,
                b->size - offset);
        memcpy((char *) b->data + offset, data, n);
    } else {
        ovs_assert(offset == b->size);
        ofpbuf_put(b, data, n);
    }