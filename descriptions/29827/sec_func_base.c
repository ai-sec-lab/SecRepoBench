cram_codec *cram_xpack_decode_init(cram_block_compression_hdr *hdr,
                                   char *data, int size,
                                   enum cram_encoding codec,
                                   enum cram_external_type option,
                                   int version, varint_vec *vv) {
    cram_codec *c;
    char *cp = data;
    char *endp = data+size;

    if (!(c = malloc(sizeof(*c))))
        return NULL;

    c->codec  = E_XPACK;
    if (option == E_LONG)
        c->decode = cram_xpack_decode_long;
    else if (option == E_INT)
        c->decode = cram_xpack_decode_int;
    else if (option == E_BYTE_ARRAY || option == E_BYTE)
        c->decode = cram_xpack_decode_char;
    else {
        fprintf(stderr, "BYTE_ARRAYs not supported by this codec\n");
        goto malformed;
    }
    c->free = cram_xpack_decode_free;
    c->size = cram_xpack_decode_size;
    c->get_block = cram_xpack_get_block;

    c->u.xpack.nbits = vv->varint_get32(&cp, endp, NULL);
    c->u.xpack.nval  = vv->varint_get32(&cp, endp, NULL);
    if (c->u.xpack.nbits >= 8  || c->u.xpack.nbits < 0 ||
        c->u.xpack.nval  > 256 || c->u.xpack.nval < 0)
        goto malformed;
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
                                             option, version, vv);
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