int pdfi_trans_setup(pdf_context *ctx, pdfi_trans_state_t *transstate, gs_rect *bbox,
                           pdfi_transparency_caller_t caller)
{
    pdfi_int_gstate *igs = (pdfi_int_gstate *)ctx->pgs->client_data;
    int code;
    bool stroked_bbox;
    bool current_overprint;
    bool okOPcs = false;
    bool ChangeBM = false;
    gs_blend_mode_t mode;
    bool need_group = false;

    memset(transstate, 0, sizeof(*transstate));

    if (!ctx->page.has_transparency)
        return 0;

    if (ctx->page.needs_OP) {
        okOPcs = pdfi_trans_okOPcs(ctx);
        if (okOPcs) {
            if (caller == TRANSPARENCY_Caller_Stroke)
                current_overprint = gs_currentstrokeoverprint(ctx->pgs);
            else {
                current_overprint = gs_currentfilloverprint(ctx->pgs);
                if (caller == TRANSPARENCY_Caller_FillStroke)
                    current_overprint |= gs_currentstrokeoverprint(ctx->pgs);
            }
            ChangeBM = current_overprint;
            mode = gs_currentblendmode(ctx->pgs);
            if (mode != BLEND_MODE_Normal && mode != BLEND_MODE_Compatible)
                need_group = ChangeBM;
            else
                need_group = false;
        } else {
            need_group = false;
        }
        need_group = need_group || (igs->SMask != NULL);
    } else {
        if (caller == TRANSPARENCY_Caller_Image || igs->SMask == NULL)
            need_group = false;
        else
            need_group = true;
    }

    code = pdfi_trans_set_params(ctx);
    if (code != 0)
        return 0;

    if (!need_group && !ChangeBM)
        return 0;

    /* TODO: error handling... */
    if (need_group) {
        bool isolated = false;
        mode = gs_currentblendmode(ctx->pgs);

        stroked_bbox = (caller == TRANSPARENCY_Caller_Stroke || caller == TRANSPARENCY_Caller_FillStroke);

        /* When changing to compatible overprint bm, the group pushed must be non-isolated. The exception
           is if we have a softmask AND the blend mode is not normal and not compatible.
           See /setupOPtrans in pdf_ops.ps  */
        if (igs->SMask != NULL && mode != BLEND_MODE_Normal && mode != BLEND_MODE_Compatible)
            isolated = true;
        // <MASK>

        transstate->saveStrokeAlpha = gs_getstrokeconstantalpha(ctx->pgs);
        transstate->saveFillAlpha = gs_getfillconstantalpha(ctx->pgs);
        code = gs_setfillconstantalpha(ctx->pgs, 1.0);
        code = gs_setstrokeconstantalpha(ctx->pgs, 1.0);
    }
    if (ChangeBM) {
        transstate->saveBM = mode;
        transstate->ChangeBM = true;
        code = gs_setblendmode(ctx->pgs, BLEND_MODE_CompatibleOverprint);
    }
    return code;
}