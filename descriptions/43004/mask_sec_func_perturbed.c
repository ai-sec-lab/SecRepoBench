static int pdfi_create_DeviceN(pdf_context *ctx, pdf_array *color_array, int index, pdf_dict *stream_dict, pdf_dict *page_dict, gs_color_space **ppcs, bool inline_image)
{
    pdf_obj *o = NULL;
    pdf_name *NamedAlternate = NULL;
    pdf_array *ArrayAlternate = NULL, *inks = NULL;
    pdf_obj *transform = NULL;
    pdf_dict *attributes = NULL;
    pdf_dict *Colorants = NULL, *Process = NULL;
    gs_color_space *process_space = NULL;
    int statuscode;
    uint64_t ix;
    gs_color_space *pcs = NULL, *pcs_alt = NULL;
    gs_function_t * pfn = NULL;

    /* Start with the array of inks */
    statuscode = pdfi_array_get_type(ctx, color_array, index + 1, PDF_ARRAY, (pdf_obj **)&inks);
    if (statuscode < 0)
        goto pdfi_devicen_error;

    for (ix = 0;ix < pdfi_array_size(inks);ix++) {
        pdf_name *ink_name = NULL;

        statuscode = pdfi_array_get_type(ctx, inks, ix, PDF_NAME, (pdf_obj **)&ink_name);
        if (statuscode < 0)
            return statuscode;

        if (ink_name->length == 3 && memcmp(ink_name->data, "All", 3) == 0) {
            pdfi_set_warning(ctx, 0, NULL, W_PDF_DEVICEN_USES_ALL, "pdfi_create_DeviceN", (char *)"WARNING: DeviceN space using /All ink name");
        }
        pdfi_countdown(ink_name);
        ink_name = NULL;
    }

    /* Sigh, Acrobat allows this, even though its contra the spec. Convert to
     * a /Separation space, and then return. Actually Acrobat does not always permit this, see
     * tests_private/comparefiles/Testform.v1.0.2.pdf.
     */
    if (pdfi_array_size(inks) == 1) {
        pdf_name *ink_name = NULL;
        pdf_array *sep_color_array = NULL;
        pdf_obj *obj = NULL;

        statuscode = pdfi_array_get_type(ctx, inks, 0, PDF_NAME, (pdf_obj **)&ink_name);
        if (statuscode < 0)
            goto pdfi_devicen_error;

        if (ink_name->length == 3 && memcmp(ink_name->data, "All", 3) == 0) {
            statuscode = pdfi_array_alloc(ctx, 4, &sep_color_array);
            if (statuscode < 0)
                goto all_error;
            pdfi_countup(sep_color_array);
            statuscode = pdfi_name_alloc(ctx, (byte *)"Separation", 10, &obj);
            if (statuscode < 0)
                goto all_error;
            pdfi_countup(obj);
            statuscode = pdfi_array_put(ctx, sep_color_array, 0, obj);
            if (statuscode < 0)
                goto all_error;
            pdfi_countdown(obj);
            obj = NULL;
            statuscode = pdfi_array_put(ctx, sep_color_array, 1, (pdf_obj *)ink_name);
            if (statuscode < 0)
                goto all_error;
            statuscode = pdfi_array_get(ctx, color_array, index + 2, &obj);
            if (statuscode < 0)
                goto all_error;
            statuscode = pdfi_array_put(ctx, sep_color_array, 2, obj);
            if (statuscode < 0)
                goto all_error;
            pdfi_countdown(obj);
            obj = NULL;
            statuscode = pdfi_array_get(ctx, color_array, index + 3, &obj);
            if (statuscode < 0)
                goto all_error;
            statuscode = pdfi_array_put(ctx, sep_color_array, 3, obj);
            if (statuscode < 0)
                goto all_error;
            pdfi_countdown(obj);
            obj = NULL;

            statuscode = pdfi_create_Separation(ctx, sep_color_array, 0, stream_dict, page_dict, ppcs, inline_image);
            if (statuscode < 0)
                goto all_error;
all_error:
            pdfi_countdown(ink_name);
            pdfi_countdown(sep_color_array);
            pdfi_countdown(obj);
            pdfi_countdown(inks);
            return statuscode;
        } else
            pdfi_countdown(ink_name);
    }

    /* Deal with alternate space */
    statuscode = pdfi_array_get(ctx, color_array, index + 2, &o);
    if (statuscode < 0)
        goto pdfi_devicen_error;

    if (o->type == PDF_NAME) {
        NamedAlternate = (pdf_name *)o;
        statuscode = pdfi_create_colorspace_by_name(ctx, NamedAlternate, stream_dict, page_dict, &pcs_alt, inline_image);
        if (statuscode < 0)
            goto pdfi_devicen_error;

    } else {
        if (o->type == PDF_ARRAY) {
            // <MASK>
        }
        else {
            statuscode = gs_error_typecheck;
            pdfi_countdown(o);
            goto pdfi_devicen_error;
        }
    }

    /* Now the tint transform */
    statuscode = pdfi_array_get(ctx, color_array, index + 3, &transform);
    if (statuscode < 0)
        goto pdfi_devicen_error;

    statuscode = pdfi_build_function(ctx, &pfn, NULL, 1, transform, page_dict);
    if (statuscode < 0)
        goto pdfi_devicen_error;

    statuscode = gs_cspace_new_DeviceN(&pcs, pdfi_array_size(inks), pcs_alt, ctx->memory);
    if (statuscode < 0)
        return statuscode;

    rc_decrement(pcs_alt, "pdfi_create_DeviceN");
    pcs_alt = NULL;
    pcs->params.device_n.mem = ctx->memory;

    for (ix = 0;ix < pdfi_array_size(inks);ix++) {
        pdf_name *ink_name;

        ink_name = NULL;
        statuscode = pdfi_array_get_type(ctx, inks, ix, PDF_NAME, (pdf_obj **)&ink_name);
        if (statuscode < 0)
            goto pdfi_devicen_error;

        pcs->params.device_n.names[ix] = (char *)gs_alloc_bytes(ctx->memory->non_gc_memory, ink_name->length + 1, "pdfi_setdevicenspace(ink)");
        memcpy(pcs->params.device_n.names[ix], ink_name->data, ink_name->length);
        pcs->params.device_n.names[ix][ink_name->length] = 0x00;
        pdfi_countdown(ink_name);
    }

    statuscode = gs_cspace_set_devn_function(pcs, pfn);
    if (statuscode < 0)
        goto pdfi_devicen_error;

    if (pdfi_array_size(color_array) >= index + 5) {
        pdf_obj *ColorSpace = NULL;
        pdf_array *Components = NULL;
        pdf_obj *subtype = NULL;

        statuscode = pdfi_array_get_type(ctx, color_array, index + 4, PDF_DICT, (pdf_obj **)&attributes);
        if (statuscode < 0)
            goto pdfi_devicen_error;

        statuscode = pdfi_dict_knownget(ctx, attributes, "Subtype", (pdf_obj **)&subtype);
        if (statuscode < 0)
            goto pdfi_devicen_error;

        if (statuscode == 0) {
            pcs->params.device_n.subtype = gs_devicen_DeviceN;
        } else {
            if (subtype->type == PDF_NAME || subtype->type == PDF_STRING) {
                if (memcmp(((pdf_name *)subtype)->data, "DeviceN", 7) == 0) {
                    pcs->params.device_n.subtype = gs_devicen_DeviceN;
                } else {
                    if (memcmp(((pdf_name *)subtype)->data, "NChannel", 8) == 0) {
                        pcs->params.device_n.subtype = gs_devicen_NChannel;
                    } else {
                        pdfi_countdown(subtype);
                        goto pdfi_devicen_error;
                    }
                }
                pdfi_countdown(subtype);
            } else {
                pdfi_countdown(subtype);
                goto pdfi_devicen_error;
            }
        }

        statuscode = pdfi_dict_knownget_type(ctx, attributes, "Process", PDF_DICT, (pdf_obj **)&Process);
        if (statuscode < 0)
            goto pdfi_devicen_error;

        if (Process != NULL && pdfi_dict_entries(Process) != 0) {
            int ix = 0;
            pdf_obj *name;

            statuscode = pdfi_dict_get(ctx, Process, "ColorSpace", (pdf_obj **)&ColorSpace);
            if (statuscode < 0)
                goto pdfi_devicen_error;

            statuscode = pdfi_create_colorspace(ctx, ColorSpace, stream_dict, page_dict, &process_space, inline_image);
            pdfi_countdown(ColorSpace);
            if (statuscode < 0)
                goto pdfi_devicen_error;

            pcs->params.device_n.devn_process_space = process_space;

            statuscode = pdfi_dict_get_type(ctx, Process, "Components", PDF_ARRAY, (pdf_obj **)&Components);
            if (statuscode < 0)
                goto pdfi_devicen_error;

            pcs->params.device_n.num_process_names = pdfi_array_size(Components);
            pcs->params.device_n.process_names = (char **)gs_alloc_bytes(pcs->params.device_n.mem->non_gc_memory, pdfi_array_size(Components) * sizeof(char *), "pdfi_devicen(Processnames)");
            if (pcs->params.device_n.process_names == NULL) {
                statuscode = gs_error_VMerror;
                goto pdfi_devicen_error;
            }

            for (ix = 0; ix < pcs->params.device_n.num_process_names; ix++) {
                statuscode = pdfi_array_get(ctx, Components, ix, &name);
                if (statuscode < 0) {
                    pdfi_countdown(Components);
                    goto pdfi_devicen_error;
                }

                if (name->type == PDF_NAME || name->type == PDF_STRING) {
                    pcs->params.device_n.process_names[ix] = (char *)gs_alloc_bytes(pcs->params.device_n.mem->non_gc_memory, ((pdf_name *)name)->length + 1, "pdfi_devicen(Processnames)");
                    if (pcs->params.device_n.process_names[ix] == NULL) {
                        pdfi_countdown(Components);
                        pdfi_countdown(name);
                        statuscode = gs_error_VMerror;
                        goto pdfi_devicen_error;
                    }
                    memcpy(pcs->params.device_n.process_names[ix], ((pdf_name *)name)->data, ((pdf_name *)name)->length);
                    pcs->params.device_n.process_names[ix][((pdf_name *)name)->length] = 0x00;
                    pdfi_countdown(name);
                } else {
                    pdfi_countdown(Components);
                    pdfi_countdown(name);
                    goto pdfi_devicen_error;
                }
            }
            pdfi_countdown(Components);
        }

        statuscode = pdfi_dict_knownget_type(ctx, attributes, "Colorants", PDF_DICT, (pdf_obj **)&Colorants);
        if (statuscode < 0)
            goto pdfi_devicen_error;

        if (Colorants != NULL && pdfi_dict_entries(Colorants) != 0) {
            uint64_t ix = 0;
            pdf_obj *Colorant = NULL, *Space = NULL;
            char *colorant_name;
            gs_color_space *colorant_space = NULL;

            statuscode = pdfi_dict_first(ctx, Colorants, &Colorant, &Space, &ix);
            if (statuscode < 0)
                goto pdfi_devicen_error;

            do {
                if (Space->type != PDF_STRING && Space->type != PDF_NAME && Space->type != PDF_ARRAY) {
                    pdfi_countdown(Space);
                    pdfi_countdown(Colorant);
                    statuscode = gs_note_error(gs_error_typecheck);
                    goto pdfi_devicen_error;
                }
                if (Colorant->type != PDF_STRING && Colorant->type != PDF_NAME) {
                    pdfi_countdown(Space);
                    pdfi_countdown(Colorant);
                    statuscode = gs_note_error(gs_error_typecheck);
                    goto pdfi_devicen_error;
                }

                statuscode = pdfi_create_colorspace(ctx, Space, stream_dict, page_dict, &colorant_space, inline_image);
                if (statuscode < 0) {
                    pdfi_countdown(Space);
                    pdfi_countdown(Colorant);
                    goto pdfi_devicen_error;
                }

                colorant_name = (char *)gs_alloc_bytes(pcs->params.device_n.mem->non_gc_memory, ((pdf_name *)Colorant)->length + 1, "pdfi_devicen(colorant)");
                if (colorant_name == NULL) {
                    rc_decrement_cs(colorant_space, "pdfi_devicen(colorant)");
                    pdfi_countdown(Space);
                    pdfi_countdown(Colorant);
                    statuscode = gs_note_error(gs_error_VMerror);
                    goto pdfi_devicen_error;
                }
                memcpy(colorant_name, ((pdf_name *)Colorant)->data, ((pdf_name *)Colorant)->length);
                colorant_name[((pdf_name *)Colorant)->length] = 0x00;

                statuscode = gs_attach_colorant_to_space(colorant_name, pcs, colorant_space, pcs->params.device_n.mem->non_gc_memory);
                if (statuscode < 0) {
                    gs_free_object(pcs->params.device_n.mem->non_gc_memory, colorant_name, "pdfi_devicen(colorant)");
                    rc_decrement_cs(colorant_space, "pdfi_devicen(colorant)");
                    pdfi_countdown(Space);
                    pdfi_countdown(Colorant);
                    statuscode = gs_note_error(gs_error_VMerror);
                    goto pdfi_devicen_error;
                }

                /* We've attached the colorant colour space to the DeviceN space, we no longer need this
                 * reference to it, so discard it.
                 */
                rc_decrement_cs(colorant_space, "pdfi_devicen(colorant)");
                pdfi_countdown(Space);
                pdfi_countdown(Colorant);
                Space = Colorant = NULL;

                statuscode = pdfi_dict_next(ctx, Colorants, &Colorant, &Space, &ix);
                if (statuscode == gs_error_undefined)
                    break;

                if (statuscode < 0) {
                    pdfi_countdown(Space);
                    pdfi_countdown(Colorant);
                    goto pdfi_devicen_error;
                }
            }while (1);
        }
    }

    if (ppcs!= NULL){
        *ppcs = pcs;
        pdfi_set_colour_callback(pcs, ctx, pdfi_cspace_free_callback);
    } else {
        statuscode = pdfi_gs_setcolorspace(ctx, pcs);
        /* release reference from construction */
        rc_decrement_only_cs(pcs, "setdevicenspace");
    }
    pdfi_countdown(Process);
    pdfi_countdown(Colorants);
    pdfi_countdown(attributes);
    pdfi_countdown(inks);
    pdfi_countdown(NamedAlternate);
    pdfi_countdown(ArrayAlternate);
    pdfi_countdown(transform);
    return_error(0);

pdfi_devicen_error:
    pdfi_free_function(ctx, pfn);
    if (pcs_alt != NULL)
        rc_decrement_only_cs(pcs_alt, "setseparationspace");
    if(pcs != NULL)
        rc_decrement_only_cs(pcs, "setseparationspace");
    pdfi_countdown(Process);
    pdfi_countdown(Colorants);
    pdfi_countdown(attributes);
    pdfi_countdown(inks);
    pdfi_countdown(NamedAlternate);
    pdfi_countdown(ArrayAlternate);
    pdfi_countdown(transform);
    return statuscode;
}