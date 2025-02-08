if (descpfont != NULL && ((pdf_cidfont_t *)descpfont)->substitute) {
        pdf_obj *csi = NULL;
        pdf_string *reg = NULL, *ord = NULL;
        char *r = NULL, *o = NULL;
        int rlen = 0, olen = 0;

        code = pdfi_dict_get(context, decfontdict, "CIDSystemInfo", (pdf_obj **)&csi);
        if (code >= 0) {
            (void)pdfi_dict_get(context, (pdf_dict *)csi, "Registry", (pdf_obj **)&reg);
            (void)pdfi_dict_get(context, (pdf_dict *)csi, "Ordering", (pdf_obj **)&ord);
            if (reg != NULL && ord != NULL) {
                r = (char *)reg->data;
                rlen = reg->length;
                o = (char *)ord->data;
                olen = ord->length;
            }
            pdfi_countdown(csi);
            pdfi_countdown(reg);
            pdfi_countdown(ord);
        }
        if (r == NULL || o == NULL) {
            r = (char *)pcmap->csi_reg.data;
            rlen = pcmap->csi_reg.size;
            o = (char *)pcmap->csi_ord.data;
            olen = pcmap->csi_ord.size;
        }
        if (rlen > 0 && olen > 0)
            pdfi_font0_cid_subst_tables(r, rlen, o, olen, &dec, &substnwp);
        else {
            dec = NULL;
            substnwp = NULL;
        }
    }
    /* reference is now owned by the descendent font created above */
    pdfi_countdown(decfontdict);
    decfontdict = NULL;
    if (code < 0) {
        code = gs_note_error(gs_error_invalidfont);
        goto error;
    }
    /* If we're got this far, we have a CMap and a descendant font, let's make the Type 0 */
    pdft0 = (pdf_font_type0 *)gs_alloc_bytes(context->memory, sizeof(pdf_font_type0), "pdfi (type0 pdf_font)");
    if (pdft0 == NULL) {
        code = gs_note_error(gs_error_VMerror);
        goto error;
    }
    code = pdfi_array_alloc(context, 1, &arr);
    if (code < 0) {
        gs_free_object(context->memory, pdft0, "pdfi_read_type0_font(pdft0)");
        goto error;
    }
    arr->refcnt = 1;
    code = pdfi_array_put(context, arr, 0, (pdf_obj *)descpfont);
    if (code < 0) {
        gs_free_object(context->memory, pdft0, "pdfi_read_type0_font(pdft0)");
        goto error;
    }

    pdft0->type = PDF_FONT;
    pdft0->pdfi_font_type = e_pdf_font_type0;
    pdft0->ctx = context;
#if REFCNT_DEBUG
    pdft0->UID = context->UID++;
    dmprintf2(context->memory, "Allocated object of type %c with UID %"PRIi64"\n", pdft0->type, pdft0->UID);
#endif
    pdft0->refcnt = 1;
    pdft0->object_num = font_dict->object_num;
    pdft0->generation_num = font_dict->generation_num;
    pdft0->indirect_num = font_dict->indirect_num;
    pdft0->indirect_gen = font_dict->indirect_gen;
    pdft0->Encoding = (pdf_obj *)pcmap;
    pdft0->ToUnicode = tounicode;
    tounicode = NULL;
    pdft0->DescendantFonts = arr;
    pdft0->PDF_font = font_dict;
    pdfi_countup(font_dict);
    pdft0->FontDescriptor = fontdesc;
    fontdesc = NULL;
    pdft0->BaseFont = basefont;
    pdft0->decoding = dec;
    pdft0->substnwp = substnwp;

    /* Ownership transferred to pdft0, if we jump to error
     * these will now be freed by counting down pdft0.
     */
    tounicode = NULL;
    arr = NULL;
    basefont = NULL;

    pdft0->pfont = NULL; /* In case we error out */

    pfont0 = (gs_font_type0 *)gs_alloc_struct(context->memory, gs_font, &st_gs_font_type0, "pdfi gs type 0 font");
    if (pfont0 == NULL) {
        gs_free_object(context->memory, pdft0, "pdfi_read_type0_font(pdft0)");
        code = gs_note_error(gs_error_VMerror);
        goto error;
    }
    gs_make_identity(&pfont0->orig_FontMatrix);
    gs_make_identity(&pfont0->FontMatrix);
    pfont0->next = pfont0->prev = 0;
    pfont0->memory = context->memory;
    pfont0->dir = context->font_dir;
    pfont0->is_resource = false;
    gs_notify_init(&pfont0->notify_list, context->memory);
    pfont0->id = gs_next_ids(context->memory, 1);
    pfont0->base = (gs_font *) pfont0;
    pfont0->client_data = pdft0;
    pfont0->WMode = pcmap->wmode;
    pfont0->FontType = ft_composite;
    pfont0->PaintType = 0;
    pfont0->StrokeWidth = 0;
    pfont0->is_cached = 0;
    if (pdft0->BaseFont != NULL) {
        pdf_name *nobj = (pdf_name *)pdft0->BaseFont;
        nlen = nobj->length > gs_font_name_max ? gs_font_name_max : nobj->length;

        memcpy(pfont0->key_name.chars, nobj->data, nlen);
        pfont0->key_name.size = nlen;
        memcpy(pfont0->font_name.chars, nobj->data, nlen);
        pfont0->font_name.size = nlen;
    }
    else {
        nlen = descpfont->pfont->key_name.size > gs_font_name_max ? gs_font_name_max : descpfont->pfont->key_name.size;

        memcpy(pfont0->key_name.chars, descpfont->pfont->key_name.chars, nlen);
        pfont0->key_name.size = nlen;
        memcpy(pfont0->font_name.chars, descpfont->pfont->font_name.chars, nlen);
        pfont0->font_name.size = nlen;
    }

    if (pcmap->name.size > 0) {
        if (pfont0->key_name.size + pcmap->name.size + 1 < gs_font_name_max) {
            memcpy(pfont0->key_name.chars + pfont0->key_name.size, "-", 1);
            memcpy(pfont0->key_name.chars + pfont0->key_name.size + 1, pcmap->name.data, pcmap->name.size);
            pfont0->key_name.size += pcmap->name.size + 1;
        }
        if (pfont0->font_name.size + pcmap->name.size + 1 < gs_font_name_max) {
            memcpy(pfont0->font_name.chars + pfont0->font_name.size, "-", 1);
            memcpy(pfont0->font_name.chars + pfont0->font_name.size + 1, pcmap->name.data, pcmap->name.size);
            pfont0->font_name.size += pcmap->name.size + 1;
        }
    }
    pfont0->procs.define_font = gs_no_define_font;
    pfont0->procs.make_font = gs_no_make_font;
    pfont0->procs.font_info = gs_default_font_info;
    pfont0->procs.same_font = gs_default_same_font;
    pfont0->procs.encode_char = pdfi_encode_char;
    pfont0->procs.decode_glyph = pdfi_font0_map_glyph_to_unicode;
    pfont0->procs.enumerate_glyph = gs_no_enumerate_glyph;
    pfont0->procs.glyph_info = gs_default_glyph_info;
    pfont0->procs.glyph_outline = gs_no_glyph_outline;
    pfont0->procs.glyph_name = pdfi_font0_glyph_name;
    pfont0->procs.init_fstack = gs_type0_init_fstack;
    pfont0->procs.next_char_glyph = gs_type0_next_char_glyph;
    pfont0->procs.build_char = gs_no_build_char;

    pfont0->data.FMapType = fmap_CMap;
    pfont0->data.EscChar = 0xff;
    pfont0->data.ShiftIn = 0x0f;
    pfont0->data.SubsVector.data = NULL;
    pfont0->data.SubsVector.size = 0;
    pfont0->data.subs_size = pfont0->data.subs_width = 0;

    pfont0->data.Encoding = (uint *)gs_alloc_bytes(context->memory, sizeof(uint), "pdfi_read_type0_font Encoding");
    if (pfont0->data.Encoding == NULL) {
        gs_free_object(context->memory, pfont0, "pdfi_read_type0_font(pfont0)");
        code = gs_note_error(gs_error_VMerror);
        goto error;
    }
    *pfont0->data.Encoding = 0;

    pfont0->data.encoding_size = 1;
    pfont0->data.FDepVector = (gs_font **)gs_alloc_bytes(context->memory, sizeof(gs_font *), "pdfi_read_type0_font FDepVector");
    if (pfont0->data.FDepVector == NULL) {
        /* We transferred ownership of pcmap to pfont0 above, but we didn't null the pointer
         * so we could keep using it. We must NULL it out before returning an error to prevent
         * reference counting problems.
         */
        pcmap = NULL;
        gs_free_object(context->memory, pfont0, "pdfi_read_type0_font(pfont0)");
        code = gs_note_error(gs_error_VMerror);
        goto error;
    }
    *pfont0->data.FDepVector = (gs_font *)descpfont->pfont;
    pdfi_countdown(descpfont);
    descpfont = NULL;
    pfont0->data.fdep_size = 1;
    pfont0->data.CMap = (gs_cmap_t *)pcmap->gscmap;

    /* NULL he pointer to prevent any reference counting problems, ownership was
     * transferred to pfont0, but we maintained the pointer for easy access until this
     * point.
     */
    pcmap = NULL;

    pdft0->pfont = (gs_font_base *)pfont0;

    code = gs_definefont(context->font_dir, (gs_font *)pdft0->pfont);
    if (code < 0) {
        gs_free_object(context->memory, pfont0, "pdfi_read_type0_font(pfont0)");
        code = gs_note_error(gs_error_VMerror);
        goto error;
    }

    /* object_num can be zero if the dictionary was defined inline */
    if (pdft0->object_num != 0) {
        code = replace_cache_entry(context, (pdf_obj *)pdft0);
        if (code < 0) {
            gs_free_object(context->memory, pfont0, "pdfi_read_type0_font(pfont0)");
            code = gs_note_error(gs_error_VMerror);
            goto error;
        }
    }

    *ppdffont = (pdf_font *)pdft0;
    return 0;

error:
    pdfi_countdown(arr);
    pdfi_countdown(pcmap);
    pdfi_countdown(tounicode);
    pdfi_countdown(basefont);
    pdfi_countdown(decfontdict);
    pdfi_countdown(dfontdesc);
    pdfi_countdown(fontdesc);
    pdfi_countdown(ffile);
    pdfi_countdown(descpfont);
    pdfi_countdown(pdft0);

    return code;