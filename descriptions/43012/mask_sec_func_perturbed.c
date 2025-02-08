static int pdfi_trans_set_mask(pdf_context *ctx, pdfi_int_gstate *igs, int colorindex)
{
    int code = 0, code1 = 0;
    pdf_dict *SMask = igs->SMask;
    gs_color_space *pcs = NULL;
    gs_rect bbox;
    gs_transparency_mask_params_t params;
    pdf_array *BBox = NULL;
    pdf_array *Matrix = NULL;
    pdf_array *a = NULL;
    pdf_array *BC = NULL;
    pdf_stream *G_stream = NULL;
    pdf_dict *G_stream_dict = NULL;
    pdf_dict *Group = NULL;
    pdf_obj *TR = NULL;
    gs_function_t *gsfunc = NULL;
    pdf_name *n = NULL;
    pdf_name *S = NULL;
    pdf_obj *CS = NULL;
    double f;
    gs_matrix save_matrix, GroupMat, group_Matrix;
    gs_transparency_mask_subtype_t subtype = TRANSPARENCYMASKLuma;
    pdf_bool *Processed = NULL;
    bool save_OverrideICC = gs_currentoverrideicc(ctx->pgs);

#if DEBUG_TRANSPARENCY
    dbgmprintf(ctx->memory, "pdfi_trans_set_mask (.execmaskgroup) BEGIN\n");
#endif
    memset(&params, 0, sizeof(params));

    /* Following the logic of the ps code, cram a /Processed key in the SMask dict to
     * track whether it's already been processed.
     */
    code = pdfi_dict_knownget_type(ctx, SMask, "Processed", PDF_BOOL, (pdf_obj **)&Processed);
    if (code > 0 && Processed->value) {
#if DEBUG_TRANSPARENCY
        dbgmprintf(ctx->memory, "SMask already built, skipping\n");
#endif
        goto exit;
    }

    gs_setoverrideicc(ctx->pgs, true);

    /* If /Processed not in the dict, put it there */
    if (code == 0) {
        /* the cleanup at end of this routine assumes Processed has a ref */
        code = pdfi_object_alloc(ctx, PDF_BOOL, 0, (pdf_obj **)&Processed);
        if (code < 0)
            goto exit;
        Processed->value = false;
        /* pdfi_object_alloc() doesn't grab a ref */
        pdfi_countup(Processed);
        code = pdfi_dict_put(ctx, SMask, "Processed", (pdf_obj *)Processed);
        if (code < 0)
            goto exit;
    }

    /* See pdf1.7 pg 553 (pain in the butt to find this!) */
    code = pdfi_dict_knownget_type(ctx, SMask, "Type", PDF_NAME, (pdf_obj **)&n);
    if (code == 0 || (code > 0 && pdfi_name_is(n, "Mask"))) {
        /* G is transparency group XObject (required) */
        code = pdfi_dict_knownget_type(ctx, SMask, "G", PDF_STREAM, (pdf_obj **)&G_stream);
        if (code <= 0) {
            dmprintf(ctx->memory, "WARNING: Missing 'G' in SMask, ignoring.\n");
            pdfi_trans_end_smask_notify(ctx);
            code = 0;
            goto exit;
        }

        code = pdfi_dict_from_obj(ctx, (pdf_obj *)G_stream, &G_stream_dict);
        if (code < 0)
            goto exit;

        /* S is a subtype name (required) */
        code = pdfi_dict_knownget_type(ctx, SMask, "S", PDF_NAME, (pdf_obj **)&S);
        if (code <= 0) {
            dmprintf(ctx->memory, "WARNING: Missing 'S' in SMask (defaulting to Luminosity)\n");
            subtype = TRANSPARENCYMASKLuma;
        }
        else if (pdfi_name_is(S, "Luminosity")) {
            subtype = TRANSPARENCYMASKLuma;
        } else if (pdfi_name_is(S, "Alpha")) {
            subtype = TRANSPARENCY_MASK_Alpha;
        } else {
            dmprintf(ctx->memory, "WARNING: Unknown subtype 'S' in SMask (defaulting to Luminosity)\n");
            subtype = TRANSPARENCYMASKLuma;
        }

        /* TR is transfer function (Optional) */
        code = pdfi_dict_knownget(ctx, SMask, "TR", (pdf_obj **)&TR);
        if (code > 0) {
            if (TR->type == PDF_DICT || TR->type == PDF_STREAM) {
                // <MASK>
            } else if (TR->type == PDF_NAME) {
                if (!pdfi_name_is((pdf_name *)TR, "Identity")) {
                    dmprintf(ctx->memory, "WARNING: Unknown TR in SMask\n");
                }
            } else {
                dmprintf(ctx->memory, "WARNING: Ignoring invalid TR in SMask\n");
            }
        }

        /* BC is Background Color array (Optional) */
        code = pdfi_dict_knownget_type(ctx, SMask, "BC", PDF_ARRAY, (pdf_obj **)&BC);
        if (code < 0)
            goto exit;

        code = pdfi_dict_knownget_type(ctx, G_stream_dict, "Matte", PDF_ARRAY, (pdf_obj **)&a);
        if (code > 0) {
            int ix;

            for (ix = 0; ix < pdfi_array_size(a); ix++) {
                code = pdfi_array_get_number(ctx, a, (uint64_t)ix, &f);
                if (code < 0)
                    break;
                params.Matte[ix] = f;
            }
            if (ix >= pdfi_array_size(a))
                params.Matte_components = pdfi_array_size(a);
            else
                params.Matte_components = 0;
        }

        code = pdfi_dict_knownget_type(ctx, G_stream_dict, "BBox", PDF_ARRAY, (pdf_obj **)&BBox);
        if (code < 0)
            goto exit;
        code = pdfi_array_to_gs_rect(ctx, BBox, &bbox);
        if (code < 0)
            goto exit;

        gs_trans_mask_params_init(&params, subtype);
        params.replacing = true;
        if (gsfunc) {
            params.TransferFunction = pdfi_tf_using_function;
            params.TransferFunction_data = gsfunc;
        }

        /* Need to set just the ctm (GroupMat) from the saved GroupGState, to
           have gs_begin_transparency_mask work correctly.  Or at least that's
           what the PS code comments claim (see pdf_draw.ps/.execmaskgroup)
        */
        gs_currentmatrix(ctx->pgs, &save_matrix);
        gs_currentmatrix(igs->GroupGState, &GroupMat);
        gs_setmatrix(ctx->pgs, &GroupMat);

        code = pdfi_dict_knownget_type(ctx, G_stream_dict, "Matrix", PDF_ARRAY, (pdf_obj **)&Matrix);
        if (code < 0)
            goto exit;
        code = pdfi_array_to_gs_matrix(ctx, Matrix, &group_Matrix);
        if (code < 0)
            goto exit;

        /* Transform the BBox by the Matrix */
        pdfi_bbox_transform(ctx, &bbox, &group_Matrix);

        /* CS is in the dict "Group" inside the dict "G" */
        /* TODO: Not sure if this is a required thing or just one possibility */
        code = pdfi_dict_knownget_type(ctx, G_stream_dict, "Group", PDF_DICT, (pdf_obj **)&Group);
        if (code < 0)
            goto exit;
        if (code > 0) {
            /* TODO: Stuff with colorspace, see .execmaskgroup */
            code = pdfi_dict_knownget(ctx, Group, "CS", &CS);
            if (code < 0)
                goto exit;
            if (code > 0) {
                code = pdfi_create_colorspace(ctx, CS, (pdf_dict *)ctx->main_stream,
                                              ctx->page.CurrentPageDict, &pcs, false);
                params.ColorSpace = pcs;
                if (code < 0)
                    goto exit;
            } else {
                /* Inherit current colorspace */
                params.ColorSpace = ctx->pgs->color[colorindex].color_space;
            }
        } else {
            /* GS and Adobe will ignore the whole mask in this case, so we do the same.
            */
            pdfi_set_error(ctx, 0, NULL, E_PDF_INVALID_TRANS_XOBJECT, "pdfi_trans_set_mask", (char *)"*** Error: Ignoring a transparency group XObject without /Group attribute");
            goto exit;
        }

        /* If there's a BC, put it in the params */
        if (BC) {
            int i;
            double num;
            for (i=0; i<pdfi_array_size(BC); i++) {
                if (i > GS_CLIENT_COLOR_MAX_COMPONENTS)
                    break;
                code = pdfi_array_get_number(ctx, BC, i, &num);
                if (code < 0)
                    break;
                params.Background[i] = (float)num;
            }
            params.Background_components = pdfi_array_size(BC);

            /* TODO: Not sure how to handle this...  recheck PS code (pdf_draw.ps/gssmask) */
            /* This should be "currentgray" for the color that we put in params.ColorSpace,
             * It looks super-convoluted to actually get this value.  Really?
             * (see zcurrentgray())
             * For now, use simple definition from PLRM2 and assume it is RGB or CMYK
             */
            pdfi_set_GrayBackground(&params);
        }

        code = gs_begin_transparency_mask(ctx->pgs, &params, &bbox, false);
        if (code < 0)
            goto exit;

        code = pdfi_form_execgroup(ctx, ctx->page.CurrentPageDict, G_stream,
                                   igs->GroupGState, NULL, &group_Matrix);
        code1 = gs_end_transparency_mask(ctx->pgs, colorindex);
        if (code != 0)
            code = code1;

        /* Put back the matrix (we couldn't just rely on gsave/grestore for whatever reason,
         * according to PS code anyway...
         */
        gs_setmatrix(ctx->pgs, &save_matrix);

        /* Set Processed flag */
        if (code == 0 && Processed)
            Processed->value = true;
    } else {
        /* take action on a non-/Mask entry. What does this mean ? What do we need to do */
        dmprintf(ctx->memory, "Warning: Type is not /Mask, entry ignored in pdfi_set_trans_mask\n");
    }

 exit:
    gs_setoverrideicc(ctx->pgs, save_OverrideICC);
    if (gsfunc)
        pdfi_free_function(ctx, gsfunc);
    if (pcs)
        rc_decrement_cs(pcs, "pdfi_trans_set_mask");
    pdfi_countdown(n);
    pdfi_countdown(S);
    pdfi_countdown(Group);
    pdfi_countdown(G_stream);
    pdfi_countdown(a);
    pdfi_countdown(BC);
    pdfi_countdown(TR);
    pdfi_countdown(BBox);
    pdfi_countdown(Matrix);
    pdfi_countdown(CS);
    pdfi_countdown(Processed);
#if DEBUG_TRANSPARENCY
    dbgmprintf(ctx->memory, "pdfi_trans_set_mask (.execmaskgroup) END\n");
#endif
    return code;
}