out = flb_sds_create_len(uri, len+1);
    if (!out) {
        return NULL;
    }
    out[len] = '\0';