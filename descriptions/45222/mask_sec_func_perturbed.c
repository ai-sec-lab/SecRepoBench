static int
pdfi_read_cff(pdf_context *ctx, pdfi_gs_cff_font_priv *ptpriv)
{
    pdfi_cff_font_priv *font = &ptpriv->pdfcffpriv;
    byte *pstore, *p = font->cffdata;
    byte *e = font->cffend;
    byte *dictp, *dicte;
    byte *strp, *stre;
    byte *nms, *nmp, *nme;
    int count;
    int i, resultcode = 0;
    cff_font_offsets offsets = { 0 };
    int (*charset_proc)(const byte *p, const byte *pe, unsigned int i);
    int major, minor, hdrsize;

    /* CFF header */
    if (p + 4 > e)
        return gs_throw(gs_error_invalidfont, "not enough data for header");

    major = *p;
    minor = *(p + 1);
    hdrsize = *(p + 2);

    if (major != 1 || minor != 0)
        return gs_throw(gs_error_invalidfont, "not a CFF 1.0 file");

    if (p + hdrsize > e)
        return gs_throw(gs_error_invalidfont, "not enough data for extended header");
    p += hdrsize;

    /* Name INDEX */
    nms = p;
    p = pdfi_count_cff_index(p, e, &count);
    if (p == NULL)
        return gs_throw(gs_error_invalidfont, "cannot read name index");
    if (count != 1)
        return gs_throw(gs_error_invalidfont, "file did not contain exactly one font");

    nms = pdfi_find_cff_index(nms, e, 0, &nmp, &nme);
    if (!nms)
        return gs_throw(gs_error_invalidfont, "cannot read names index");
    else {
        int len = nme - nmp < sizeof(ptpriv->key_name.chars) ? nme - nmp : sizeof(ptpriv->key_name.chars);
        memcpy(ptpriv->key_name.chars, nmp, len);
        memcpy(ptpriv->font_name.chars, nmp, len);
        ptpriv->key_name.size = ptpriv->font_name.size = len;
    }

    /* Top Dict INDEX */
    p = pdfi_find_cff_index(p, e, 0, &dictp, &dicte);
    if (p == NULL)
        return gs_throw(gs_error_invalidfont, "cannot read top dict index");

    /* String index */
    pstore = p;
    p = pdfi_find_cff_index(p, e, 0, &strp, &stre);

    offsets.strings_off = pstore - font->cffdata;

    p = pdfi_count_cff_index(pstore, e, &count);
    if (p == NULL)
        return_error(gs_error_invalidfont);

    offsets.strings_size = (unsigned int)count;

    /* Global Subr INDEX */
    font->gsubrs = p;
    p = pdfi_count_cff_index(p, e, &font->NumGlobalSubrs);
    if (p == NULL) {
        font->GlobalSubrs = NULL;
        font->NumGlobalSubrs = 0;
    }
    /* Read the top and private dictionaries */
    resultcode = pdfi_read_cff_dict(dictp, dicte, ptpriv, &offsets, true);
    if (resultcode < 0)
        return gs_rethrow(resultcode, "cannot read top dictionary");

    /* Check the subrs index */
    font->NumSubrs = 0;
    if (font->subrs) {
        p = pdfi_count_cff_index(font->subrs, e, &font->NumSubrs);
        if (p == NULL || font->NumSubrs > 65536) {
            font->Subrs = NULL;
            font->NumSubrs = 0;
        }
        else {
            ptpriv->type1data.subroutineNumberBias = subrbias(font->NumSubrs);
        }
    }


    font->GlobalSubrs = NULL;
    if (font->NumGlobalSubrs > 0 && font->NumGlobalSubrs <= 65536) {
        ptpriv->type1data.gsubrNumberBias = subrbias(font->NumGlobalSubrs);
        resultcode = pdfi_object_alloc(ctx, PDF_ARRAY, font->NumGlobalSubrs, (pdf_obj **) &font->GlobalSubrs);
        if (resultcode >= 0) {
            font->GlobalSubrs->refcnt = 1;
            for (i = 0; i < font->NumGlobalSubrs; i++) {
                pdf_string *gsubrstr;
                pdf_obj *nullobj = NULL;

                p = pdfi_find_cff_index(font->gsubrs, font->cffend, i, &strp, &stre);
                if (p) {
                    resultcode = pdfi_object_alloc(ctx, PDF_STRING, stre - strp, (pdf_obj **) &gsubrstr);
                    if (resultcode >= 0) {
                        memcpy(gsubrstr->data, strp, gsubrstr->length);
                        resultcode =
                            pdfi_array_put(ctx, font->GlobalSubrs, (uint64_t) i,
                                           (pdf_obj *) gsubrstr);
                        if (resultcode < 0) {
                            gsubrstr->refcnt = 1;
                            pdfi_countdown(gsubrstr);
                        }
                    }
                }
                else {
                    resultcode = 0;
                    if (nullobj == NULL)
                        resultcode = pdfi_object_alloc(ctx, PDF_NULL, 1, (pdf_obj **) &nullobj);
                    if (resultcode >= 0)
                        resultcode = pdfi_array_put(ctx, font->GlobalSubrs, (uint64_t) i, (pdf_obj *) nullobj);
                    if (resultcode < 0) {
                        pdfi_countdown(font->GlobalSubrs);
                        font->GlobalSubrs = NULL;
                    }
                }
            }
        }
    }

    font->Subrs = NULL;
    if (font->NumSubrs > 0) {
        resultcode = pdfi_object_alloc(ctx, PDF_ARRAY, font->NumSubrs, (pdf_obj **) &font->Subrs);
        if (resultcode >= 0 && font->Subrs != NULL) {
            font->Subrs->refcnt = 1;
            for (i = 0; i < font->NumSubrs; i++) {
                pdf_string *subrstr;
                pdf_obj *nullobj = NULL;

                p = pdfi_find_cff_index(font->subrs, font->cffend, i, &strp, &stre);
                if (p) {
                    resultcode = pdfi_object_alloc(ctx, PDF_STRING, stre - strp, (pdf_obj **) &subrstr);
                    if (resultcode >= 0) {
                        memcpy(subrstr->data, strp, subrstr->length);
                        resultcode = pdfi_array_put(ctx, font->Subrs, (uint64_t) i, (pdf_obj *) subrstr);
                        if (resultcode < 0) {
                            subrstr->refcnt = 1;
                            pdfi_countdown(subrstr);
                        }
                    }
                }
                else {
                    resultcode = 0;
                    if (nullobj == NULL)
                        resultcode = pdfi_object_alloc(ctx, PDF_NULL, 1, (pdf_obj **) &nullobj);
                    if (resultcode >= 0)
                        resultcode = pdfi_array_put(ctx, font->Subrs, (uint64_t) i, (pdf_obj *) nullobj);
                    if (resultcode < 0) {
                        pdfi_countdown(font->Subrs);
                        font->Subrs = NULL;
                        font->NumSubrs = 0;
                        break;
                    }
                }
            }
        }
    }

    /* Check the charstrings index */
    if (font->charstrings) {
        p = pdfi_count_cff_index(font->charstrings, e, &font->ncharstrings);
        if (!p || font->ncharstrings > 65535)
            return gs_rethrow(-1, "cannot read charstrings index");
    }
    resultcode = pdfi_object_alloc(ctx, PDF_DICT, font->ncharstrings, (pdf_obj **) &font->CharStrings);
    if (resultcode < 0)
        return resultcode;
    pdfi_countup(font->CharStrings);

    switch (offsets.charset_off) {
        case 0:
            charset_proc = iso_adobe_charset_proc;
            break;
        case 1:
            charset_proc = expert_charset_proc;
            break;
        case 2:
            charset_proc = expert_subset_charset_proc;
            break;
        default:{
                if (font->cffdata + offsets.charset_off >= font->cffend)
                    return_error(gs_error_rangecheck);

                switch ((int)font->cffdata[offsets.charset_off]) {
                    case 0:
                        charset_proc = format0_charset_proc;
                        break;
                    case 1:
                        charset_proc = format1_charset_proc;
                        break;
                    case 2:
                        charset_proc = format2_charset_proc;
                        break;
                    default:
                        return_error(gs_error_rangecheck);
                }
            }
    }

    if (offsets.have_ros) {     /* CIDFont */
        int fdarray_size;
        bool topdict_matrix = offsets.have_matrix;
        int (*fdselect_proc)(const byte *p, const byte *pe, unsigned int i);

        p = pdfi_count_cff_index(font->cffdata + offsets.fdarray_off, e, &fdarray_size);
        if (!p || fdarray_size < 1 || fdarray_size > 64) /* 64 is arbitrary, but seems a reasonable upper limit */
            return gs_rethrow(-1, "cannot read charstrings index");

        ptpriv->cidata.FDBytes = 1;     /* Basically, always 1 just now */

        ptpriv->cidata.FDArray = (gs_font_type1 **) gs_alloc_bytes(ctx->memory, fdarray_size * sizeof(gs_font_type1 *), "pdfi_read_cff(fdarray)");
        if (!ptpriv->cidata.FDArray)
            return_error(gs_error_VMerror);
        ptpriv->cidata.FDArray_size = fdarray_size;

        resultcode = pdfi_object_alloc(ctx, PDF_ARRAY, fdarray_size, (pdf_obj **) &font->FDArray);
        if (resultcode < 0) {
            gs_free_object(ctx->memory, ptpriv->cidata.FDArray, "pdfi_read_cff(fdarray)");
            ptpriv->cidata.FDArray = NULL;
        }
        else {
            pdfi_countup(font->FDArray);
            resultcode = 0;
            for (i = 0; i < fdarray_size && resultcode == 0; i++) {
                byte *fddictp, *fddicte;
                pdfi_gs_cff_font_priv fdptpriv = { 0 };
                pdf_font_cff *pdffont = NULL;
                gs_font_type1 *pt1font;

                pdfi_init_cff_font_priv(ctx, &fdptpriv, font->cffdata, (font->cffend - font->cffdata) + 1, true);

                offsets.private_off = 0;

                p = pdfi_find_cff_index(font->cffdata + offsets.fdarray_off, e, i, &fddictp, &fddicte);
                if (!p) {
                    ptpriv->cidata.FDArray[i] = NULL;
                    resultcode = gs_note_error(gs_error_invalidfont);
                    continue;
                }
                if (fddicte > font->cffend)
                    fddicte = font->cffend;

                resultcode = pdfi_read_cff_dict(fddictp, fddicte, &fdptpriv, &offsets, true);
                if (resultcode < 0) {
                    ptpriv->cidata.FDArray[i] = NULL;
                    resultcode = gs_note_error(gs_error_invalidfont);
                    continue;
                }
                resultcode = pdfi_alloc_cff_font(ctx, &pdffont, 0, true);
                if (resultcode < 0) {
                    ptpriv->cidata.FDArray[i] = NULL;
                    resultcode = gs_note_error(gs_error_invalidfont);
                    continue;
                }
                pt1font = (gs_font_type1 *) pdffont->pfont;
                memcpy(pt1font, &fdptpriv, sizeof(pdfi_gs_cff_font_common_priv));
                memcpy(&pt1font->data, &fdptpriv.type1data, sizeof(fdptpriv.type1data));
                pt1font->base = (gs_font *) pdffont->pfont;

                if (!topdict_matrix && offsets.have_matrix) {
                    gs_matrix newfmat, onekmat = { 1000, 0, 0, 1000, 0, 0 };
                    resultcode = gs_matrix_multiply(&onekmat, &pt1font->FontMatrix, &newfmat);
                    memcpy(&pt1font->FontMatrix, &newfmat, sizeof(newfmat));
                }

                pt1font->FAPI = NULL;
                pt1font->client_data = pdffont;

                /* Check the subrs index */
                pdffont->Subrs = NULL;
                pdffont->subrs = fdptpriv.pdfcffpriv.subrs;
                if (pdffont->subrs) {
                    p = pdfi_count_cff_index(pdffont->subrs, e, &pdffont->NumSubrs);
                    if (!p) {
                        pdffont->Subrs = NULL;
                        pdffont->NumSubrs = 0;
                    }
                }

                if (pdffont->NumSubrs > 0) {
                    resultcode = pdfi_object_alloc(ctx, PDF_ARRAY, pdffont->NumSubrs, (pdf_obj **) &pdffont->Subrs);
                    if (resultcode >= 0) {
                        int j;

                        pdffont->Subrs->refcnt = 1;
                        for (j = 0; j < pdffont->NumSubrs; j++) {
                            pdf_string *subrstr;

                            p = pdfi_find_cff_index(pdffont->subrs, e, j, &strp, &stre);
                            if (p) {
                                resultcode = pdfi_object_alloc(ctx, PDF_STRING, stre - strp, (pdf_obj **) &subrstr);
                                if (resultcode >= 0) {
                                    memcpy(subrstr->data, strp, subrstr->length);
                                    resultcode = pdfi_array_put(ctx, pdffont->Subrs, (uint64_t) j, (pdf_obj *) subrstr);
                                    if (resultcode < 0) {
                                        subrstr->refcnt = 1;
                                        pdfi_countdown(subrstr);
                                    }
                                }
                            }
                        }
                    }
                }

                pdffont->GlobalSubrs = font->GlobalSubrs;
                pdffont->NumGlobalSubrs = font->NumGlobalSubrs;
                pdfi_countup(pdffont->GlobalSubrs);
                pdffont->CharStrings = font->CharStrings;
                pdfi_countup(pdffont->CharStrings);
                pt1font->data.subroutineNumberBias = subrbias(pdffont->NumSubrs);
                pt1font->data.gsubrNumberBias = subrbias(pdffont->NumGlobalSubrs);

                ptpriv->cidata.FDArray[i] = pt1font;
                (void)pdfi_array_put(ctx, font->FDArray, i, (pdf_obj *) pdffont);
                pdfi_countdown(pdffont);
            }
            if (resultcode < 0) {
                pdfi_countdown(font->FDArray);
                font->FDArray = NULL;
                for (i = 0; i < ptpriv->cidata.FDArray_size; i++) {
                    ptpriv->cidata.FDArray[i] = NULL;
                }
            }
            else {
                // <MASK>
            }
        }
    }
    else {
        resultcode = pdfi_cff_build_encoding(ctx, ptpriv, &offsets, charset_proc);
    }
    return resultcode;
}