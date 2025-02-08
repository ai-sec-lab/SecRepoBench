if (pdfi_count_stack(ctx) < 1) {
        code = gs_note_error(gs_error_stackunderflow);
        goto cleanupExit;
    }

    if (pcs->type == &gs_color_space_type_Pattern)
        is_pattern = true;
    if (is_pattern) {
        if (ctx->stack_top[-1]->type != PDF_NAME) {
            pdfi_clearstack(ctx);
            code = gs_note_error(gs_error_syntaxerror);
            goto cleanupExit;
        }
        base_space = pcs->base_space;
        code = pdfi_pattern_set(ctx, stream_dict, page_dict, (pdf_name *)ctx->stack_top[-1], &cc);
        pdfi_pop(ctx, 1);
        if (code < 0) {
            /* Ignore the pattern if we failed to set it */
            pdfi_set_warning(ctx, 0, NULL, W_PDF_BADPATTERN, "pdfi_setcolorN", (char *)"PATTERN: Error setting pattern");
            code = 0;
            goto cleanupExit;
        }
        if (base_space && pattern_instance_uses_base_space(cc.pattern))
            ncomps = cs_num_components(base_space);
        else
            ncomps = 0;
    } else {
        ncomps = cs_num_components(pcs);
        cc.pattern = NULL;
    }

    if (ncomps > 0)
        code = pdfi_get_color_from_stack(ctx, &cc, ncomps);
    if (code < 0)
        goto cleanupExit;

    if (pcs->type == &gs_color_space_type_Indexed) {
        if (cc.paint.values[0] < 0)
            cc.paint.values[0] = 0.0;
        else
        {
            if (cc.paint.values[0] > pcs->params.indexed.hival)
                cc.paint.values[0] = (float)pcs->params.indexed.hival;
            else
            {
                if (cc.paint.values[0] != floor(cc.paint.values[0]))
                {
                    if (cc.paint.values[0] - floor(cc.paint.values[0]) < 0.5)
                        cc.paint.values[0] = floor(cc.paint.values[0]);
                    else
                        cc.paint.values[0] = ceil(cc.paint.values[0]);
                }
            }
        }
    }

    code = gs_setcolor(ctx->pgs, &cc);

cleanupExit:
    if (is_pattern)
        /* cc is a local scope variable, holding a reference to a pattern.
         * We need to count the refrence down before the variable goes out of scope
         * in order to prevent the pattern leaking.
         */
        rc_decrement(cc.pattern, "pdfi_setcolorN");

    if (!is_fill)
        gs_swapcolors_quick(ctx->pgs);