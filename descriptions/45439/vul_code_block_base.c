if (code >= 0 && charstring->length > gscidfont->cidata.FDBytes) {
            if (gscidfont->cidata.FDBytes != 0) {
                if ((int)charstring->data[0] > gscidfont->cidata.FDArray_size)
                    code = gs_note_error(gs_error_invalidfont);
                else
                    *pfidx = (int)charstring->data[0];
            }

            if (code >= 0 && pgd && ((int64_t)charstring->length - (int64_t)gscidfont->cidata.FDBytes) >= 0)
                gs_glyph_data_from_bytes(pgd, charstring->data + gscidfont->cidata.FDBytes, 0, charstring->length - gscidfont->cidata.FDBytes, NULL);
        }