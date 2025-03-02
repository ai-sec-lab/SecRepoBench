if (k->via.str.size != slen) {
            continue;
        }

        /* Ensure the pointer we are about to read is not NULL */
        if (k->via.str.ptr == NULL) {
            flb_free(mp_buf);
            flb_free(tmp_out_buf);
            *out_buf = NULL;
            msgpack_unpacked_destroy(&result);
            return -1;
        }