if (cs->values[0].masked) {
            lexer_error(ctx->lexer, "Only == and != operators may be used "
                        "with masked constants.  Consider using subfields "
                        "instead (e.g. eth.src[0..15] > 0x1111 in place of "
                        "eth.src > 00:00:00:00:11:11/00:00:00:00:ff:ff).");
            goto exit;
        }