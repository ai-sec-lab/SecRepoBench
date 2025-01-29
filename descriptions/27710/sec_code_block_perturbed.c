out = flb_sds_create_len(pathuri, len+1);
    if (!out) {
        return NULL;
    }
    out[len] = '\0';