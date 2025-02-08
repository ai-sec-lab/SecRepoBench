pdfi_countup(s);
    pdfi_pop(ctx, 1);

    /* Save the CTM for later restoration */
    saved = ctm_only(ctx->pgs);
    gs_currentpoint(ctx->pgs, &initial_point);

    Trm.xx = ctx->pgs->PDFfontsize * (ctx->pgs->texthscaling / 100);
    Trm.xy = 0;
    Trm.yx = 0;
    Trm.yy = ctx->pgs->PDFfontsize;
    Trm.tx = 0;
    Trm.ty = ctx->pgs->textrise;

    gs_matrix_multiply(&Trm, &ctx->pgs->textmatrix, &Trm);

    if (!ctx->device_state.preserve_tr_mode) {
        gs_distance_transform_inverse(ctx->pgs->line_params.half_width, 0, &Trm, &pt);
        ctx->pgs->line_params.half_width = sqrt((pt.x * pt.x) + (pt.y * pt.y));
    } else {
        /* We have to adjust the stroke width for pdfwrite so that we take into
         * account the CTM, but we do not spply the font scaling. Because of
         * the disconnect between pdfwrite and the interpreter, we also have to
         * remove the scaling due to the resolution.
         */
        gs_matrix devmatrix, matrix;
        gx_device *device = gs_currentdevice(ctx->pgs);

        devmatrix.xx = 72.0 / device->HWResolution[0];
        devmatrix.xy = 0;
        devmatrix.yx = 0;
        devmatrix.yy = 72.0 / device->HWResolution[1];
        devmatrix.tx = 0;
        devmatrix.ty = 0;

        code = gs_matrix_multiply(&saved, &devmatrix, &matrix);
        if (code < 0)
            goto exit;

        gs_distance_transform(ctx->pgs->line_params.half_width, 0, &matrix, &pt);
        ctx->pgs->line_params.half_width = sqrt((pt.x * pt.x) + (pt.y * pt.y));
    }

    gs_matrix_multiply(&Trm, &ctm_only(ctx->pgs), &Trm);
    gs_setmatrix(ctx->pgs, &Trm);

    code = gs_moveto(ctx->pgs, 0, 0);
    if (code < 0)
        goto Tj_error;

    code = pdfi_show(ctx, s);

    ctx->pgs->line_params.half_width = linewidth;
    /* Update the Text matrix with the current point, for the next operation
     */
    gs_currentpoint(ctx->pgs, &currpt);
    Trm.xx = ctx->pgs->PDFfontsize * (ctx->pgs->texthscaling / 100);
    Trm.xy = 0;
    Trm.yx = 0;
    Trm.yy = ctx->pgs->PDFfontsize;
    Trm.tx = 0;
    Trm.ty = 0;
    gs_matrix_multiply(&Trm, &ctx->pgs->textmatrix, &Trm);

    gs_distance_transform(currpt.x, currpt.y, &Trm, &pt);
    ctx->pgs->textmatrix.tx += pt.x;
    ctx->pgs->textmatrix.ty += pt.y;

Tj_error:
    /* Restore the CTM to the saved value */
    gs_setmatrix(ctx->pgs, &saved);
    /* And restore the currentpoint */
    gs_moveto(ctx->pgs, initial_point.x, initial_point.y);
    /* And the line width */
    ctx->pgs->line_params.half_width = linewidth;

 exit:
    pdfi_countdown(s);