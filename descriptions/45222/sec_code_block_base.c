if (font->cffdata + offsets.fdselect_off > font->cffend)
                    return_error(gs_error_rangecheck);

                switch ((int)font->cffdata[offsets.fdselect_off]) {
                    case 0:
                        fdselect_proc = format0_fdselect_proc;
                        break;
                    case 3:
                        fdselect_proc = format3_fdselect_proc;
                        break;
                    default:
                        return_error(gs_error_rangecheck);
                }

                if (font->ncharstrings > 0) {
                    int maxcid = 0;
                    for (i = 0; i < font->ncharstrings; i++) {
                        int fd, g;
                        char gkey[64];
                        pdf_string *charstr;

                        fd = fdarray_size <= 1 ? 0 : (*fdselect_proc) (font->cffdata + offsets.fdselect_off + 1, font->cffend, i);

                        p = pdfi_find_cff_index(font->charstrings, font->cffend, i, &strp, &stre);
                        if (!p)
                            continue;

                        code = pdfi_object_alloc(ctx, PDF_STRING, (stre - strp) + 1, (pdf_obj **) &charstr);
                        if (code < 0)
                            continue;
                        charstr->data[0] = (byte) fd;
                        memcpy(charstr->data + 1, strp, charstr->length - 1);

                        if (i == 0) {
                            g = 0;
                        }
                        else {
                            g = (*charset_proc) (font->cffdata + offsets.charset_off + 1, font->cffend, i - 1);
                        }

                        if (g > maxcid) maxcid = g;
                        gs_snprintf(gkey, sizeof(gkey), "%d", g);
                        code = pdfi_dict_put(ctx, font->CharStrings, gkey, (pdf_obj *) charstr);
                    }
                    if (maxcid > ptpriv->pdfcffpriv.cidcount - 1)
                        ptpriv->pdfcffpriv.cidcount = maxcid + 1;
                }