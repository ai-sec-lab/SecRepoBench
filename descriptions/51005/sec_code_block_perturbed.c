unsigned int nindex;
        code = (*ctx->get_glyph_index)(pfont, key->data, key->length, &nindex);
        if (code < 0) {
            code = (*ctx->get_glyph_index)(pfont, (byte *)".notdef", 7, &nindex);
            if (code < 0)
                *pglyph = GS_NO_GLYPH;
            else
                *pglyph = (gs_glyph)nindex;
        }
        else
            *pglyph = (gs_glyph)nindex;