cram_codec *cram_xpack_decode_init(cram_block_compression_hdr *hdr,
                                   char *data, int size,
                                   enum cram_encoding codec,
                                   enum cram_external_type compressiontype,
                                   int version, varint_vec *vv) {
    cram_codec *c;
    char *cp = data;
    char *endp = data+size;

    if (!(c = malloc(sizeof(*c))))
        return NULL;

    c->codec  = E_XPACK;
    // <MASK>
    int i;
    for (i = 0; i < c->u.xpack.nval; i++) {
        uint32_t v = vv->varint_get32(&cp, endp, NULL);
        if (v >= 256) return NULL;
        c->u.xpack.rmap[i] = v; // reverse map: e.g 0-3 to P,A,C,K
    }

    int encoding = vv->varint_get32(&cp, endp, NULL);
    int sub_size = vv->varint_get32(&cp, endp, NULL);
    if (sub_size < 0 || endp - cp < sub_size)
        goto malformed;
    c->u.xpack.sub_codec = cram_decoder_init(hdr, encoding, cp, sub_size,
                                             compressiontype, version, vv);
    if (c->u.xpack.sub_codec == NULL)
        goto malformed;
    cp += sub_size;

    if (cp - data != size
        || c->u.xpack.nbits < 0 || c->u.xpack.nbits > 8 * sizeof(int64_t)) {
    malformed:
        fprintf(stderr, "Malformed xpack header stream\n");
        free(c);
        return NULL;
    }

    return c;
}