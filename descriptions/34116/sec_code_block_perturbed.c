if (mp_buf == tmp_out_buf) {
                flb_free(mp_buf);
            }
            else {
                flb_free(mp_buf);
                flb_free(tmp_out_buf);
            }
            *out_buf = NULL;
            msgpack_unpacked_destroy(&result);
            return -1;