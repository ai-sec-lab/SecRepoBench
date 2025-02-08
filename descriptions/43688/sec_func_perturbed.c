int pdfi_page_render(pdf_context *ctx, uint64_t page_num, bool init_graphics)
{
    int code, code1=0;
    pdf_dict *page_dict = NULL;
    bool page_group_known = false;
    pdf_dict *group_dict = NULL;
    bool page_dict_error = false;
    bool need_pdf14 = false; /* true if the device is needed and was successfully pushed */
    int transparencydepth = 0; /* -1 means special mode for transparency simulation */

    if (page_num > ctx->num_pages)
        return_error(gs_error_rangecheck);

    if (ctx->args.pdfdebug)
        dmprintf1(ctx->memory, "%% Processing Page %"PRIi64" content stream\n", page_num + 1);

    code = pdfi_page_get_dict(ctx, page_num, &page_dict);
    if (code < 0) {
        char extra_info[256];

        page_dict_error = true;
        gs_sprintf(extra_info, "*** ERROR: Page %ld has invalid Page dict, skipping\n", page_num+1);
        pdfi_set_error(ctx, 0, NULL, E_PDF_PAGEDICTERROR, "pdfi_page_render", extra_info);
        if (code != gs_error_VMerror && !ctx->args.pdfstoponerror)
            code = 0;
        goto exit3;
    }

    pdfi_device_set_flags(ctx);

    code = pdfi_check_page(ctx, page_dict, init_graphics);
    if (code < 0)
        goto exit3;

    if (ctx->args.pdfdebug) {
        dbgmprintf2(ctx->memory, "Current page %ld transparency setting is %d", page_num+1,
                ctx->page.has_transparency);

        if (ctx->device_state.spot_capable)
            dbgmprintf1(ctx->memory, ", spots=%d\n", ctx->page.num_spots);
        else
            dbgmprintf(ctx->memory, "\n");
    }

    code = pdfi_dict_knownget_type(ctx, page_dict, "Group", PDF_DICT, (pdf_obj **)&group_dict);
    if (code < 0)
        goto exit3;
    if (group_dict != NULL)
        page_group_known = true;

    pdfi_countdown(ctx->page.CurrentPageDict);
    ctx->page.CurrentPageDict = page_dict;
    pdfi_countup(ctx->page.CurrentPageDict);

    /* In case we don't call pdfi_set_media_size, which sets this up.
     * We shouldn't ever use it in that case, but best to be safe.
     */
    ctx->page.UserUnit = 1.0f;
    /* If we are being called from the PDF interpreter then
     * we need to set up the page  and the default graphics state
     * but if we are being called from PostScript we do *not*
     * want to alter any of the graphics state or the media size.
     */
    /* TODO: I think this is a mix of things we might need to
     * still be setting up.
     * (for example, I noticed the blendmode and moved it outside the if)
     */
    if (init_graphics) {
        code = pdfi_set_media_size(ctx, page_dict);
        if (code < 0)
            goto exit2;

        pdfi_set_ctm(ctx);

    } else {
        /* Gets ctx->page.Size setup correctly
         * TODO: Probably not right if the page is rotated?
         * page.Size is needed by the transparency code,
         * not sure where else it might be used, if anywhere.
         */
        pdfi_get_media_size(ctx, page_dict);
    }

    /* Write the various CropBox, TrimBox etc to the device */
    pdfi_write_boxes_pdfmark(ctx, page_dict);

    code = setup_page_DefaultSpaces(ctx, page_dict);
    if (code < 0)
        goto exit2;

    pdfi_setup_transfers(ctx);

    /* Set whether device needs OP support
     * This needs to be before transparency device is pushed, if applicable
     */
    pdfi_trans_set_needs_OP(ctx);
    pdfi_oc_init(ctx);

    code = pdfi_gsave(ctx);
    if (code < 0)
        goto exit2;

    /* Figure out if pdf14 device is needed.
     * This can be either for normal transparency deviceN, or because we are using
     * Overprint=/simulate for other devices
     */
    if (ctx->page.has_transparency) {
        need_pdf14 = true;
        if (ctx->page.simulate_op)
            transparencydepth = -1;
    } else {
        /* This is the case where we are simulating overprint without transparency */
        if (ctx->page.simulate_op) {
            need_pdf14 = true;
            transparencydepth = -1;
        }
    }
    if (need_pdf14) {
        /* We don't retain the PDF14 device */
        code = gs_push_pdf14trans_device(ctx->pgs, false, false, transparencydepth, ctx->page.num_spots);
        if (code >= 0) {
            if (page_group_known) {
                code = pdfi_trans_begin_page_group(ctx, page_dict, group_dict);
                /* If setting the page group failed for some reason, abandon the page group,
                 *  but continue with the page
                 */
                if (code < 0)
                    page_group_known = false;
            }
        } else {
            /* Couldn't push the transparency compositor.
             * This is probably fatal, but attempt to recover by abandoning transparency
             */
            ctx->page.has_transparency = false;
            need_pdf14 = false;
        }
    }

    /* Init a base_pgs graphics state for Patterns
     * (this has to be after transparency device pushed, if applicable)
     */
    pdfi_set_DefaultQState(ctx, ctx->pgs);

    /* Render one page (including annotations) */
    if (!ctx->args.QUIET)
        outprintf(ctx->memory, "Page %"PRId64"\n", page_num + 1);

    code = pdfi_process_one_page(ctx, page_dict);

    if (ctx->page.has_transparency && page_group_known) {
        code1 = pdfi_trans_end_group(ctx);
    }

    if (need_pdf14) {
        if (code1 < 0) {
            (void)gs_abort_pdf14trans_device(ctx->pgs);
            goto exit1;
        }

        code = gs_pop_pdf14trans_device(ctx->pgs, false);
        if (code < 0) {
            goto exit1;
        }
    }

exit1:
    pdfi_free_DefaultQState(ctx);
    pdfi_grestore(ctx);

exit2:
    pdfi_countdown(ctx->page.CurrentPageDict);
    ctx->page.CurrentPageDict = NULL;

exit3:
    pdfi_countdown(page_dict);
    pdfi_countdown(group_dict);

    release_page_DefaultSpaces(ctx);

    if (code == 0 || (!ctx->args.pdfstoponerror && code != gs_error_stackoverflow))
        if (!page_dict_error && ctx->finish_page != NULL)
            code = ctx->finish_page(ctx);
    return code;
}