struct ofpbuf b;
            char buf[512];
            size_t mdlen, padding;
            if (ovs_scan_len(s, &n, "md2=0x%511[0-9a-fA-F]", buf)) {
                ofpbuf_use_stub(&b, metadata,
                                NSH_CTX_HDRS_MAX_LEN);
                ofpbuf_put_hex(&b, buf, &mdlen);
                /* Pad metadata to 4 bytes. */
                padding = PAD_SIZE(mdlen, 4);
                if (padding > 0) {
                    ofpbuf_push_zeros(&b, padding);
                }
                md_size = mdlen + padding;
                ofpbuf_uninit(&b);
                continue;
            }