if (k->via.str.size != slen) {
            continue;
        }

        /* Ensure the pointer we are about to read is not NULL */
        if (k->via.str.ptr == NULL) {
            if (mp_buf == tmp_out_buf) {
                flb_free(mp_buf);
            }
            else {
                flb_free(mp_buf);
                flb_free(tmp_out_buf);
            }
            *out_buf = NULL;
            msgpack_unpacked_destroy(&unpacked_data);
            return -1;
        }