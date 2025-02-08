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