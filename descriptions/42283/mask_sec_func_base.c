static	int
pdf14_put_image(gx_device * dev, gs_gstate * pgs, gx_device * target)
{
    const pdf14_device * pdev = (pdf14_device *)dev;
    int code;
    gs_image1_t image;
    gx_image_enum_common_t *info;
    pdf14_buf *buf = pdev->ctx->stack;
    gs_int_rect rect;
    int y;
    int num_comp;
    byte *linebuf, *linebuf_unaligned;
    gs_color_space *pcs;
    int x1, y1, width, height;
    byte *buf_ptr;
    int num_rows_left;
    cmm_profile_t* src_profile = NULL;
    cmm_profile_t* des_profile = NULL;
    cmm_dev_profile_t *pdf14dev_profile;
    cmm_dev_profile_t *dev_target_profile;
    uint16_t bg;
    bool has_tags = device_encodes_tags(dev);
    bool deep = pdev->ctx->deep;
    int planestride;
    int rowstride;
    blend_image_row_proc_t blend_row;
    bool color_mismatch = false;
    bool supports_alpha = false;
    int i;
    int alpha_offset, tag_offset;
    const byte* buf_ptrs[GS_CLIENT_COLOR_MAX_COMPONENTS];
    // <MASK>
    if (code < 0) {
        rc_decrement_only_cs(pcs, "pdf14_put_image");
        return code;
    }
#if RAW_DUMP
    /* Dump the current buffer to see what we have. */
    dump_raw_buffer(pdev->ctx->memory,
                    pdev->ctx->stack->rect.q.y-pdev->ctx->stack->rect.p.y,
                    pdev->ctx->stack->rect.q.x-pdev->ctx->stack->rect.p.x,
                    pdev->ctx->stack->n_planes,
                    pdev->ctx->stack->planestride, pdev->ctx->stack->rowstride,
                    "pdF14_putimage", pdev->ctx->stack->data, deep);
    dump_raw_buffer(pdev->ctx->memory,
                    height, width, buf->n_planes,
                    pdev->ctx->stack->planestride, pdev->ctx->stack->rowstride,
                    "PDF14_PUTIMAGE_SMALL", buf_ptr, deep);
    global_index++;
    clist_band_count++;
#endif
    /* Allocate on 32-byte border for AVX CMYK case. Four byte overflow for RGB case */
    /* 28 byte overflow for AVX CMYK case. */
#define SSE_ALIGN 32
#define SSE_OVERFLOW 28
    linebuf_unaligned = gs_alloc_bytes(pdev->memory, width * (num_comp<<deep) + SSE_ALIGN + SSE_OVERFLOW, "pdf14_put_image");
    if (linebuf_unaligned == NULL)
        return gs_error_VMerror;
    linebuf = linebuf_unaligned + ((-(intptr_t)linebuf_unaligned) & (SSE_ALIGN-1));

    blend_row = deep ? gx_build_blended_image_row16 :
                       gx_build_blended_image_row;
#ifdef WITH_CAL
    blend_row = cal_get_blend_row(pdev->memory->gs_lib_ctx->core->cal_ctx,
                                  blend_row, num_comp, deep);
#endif

    if (!deep)
        bg >>= 8;
    for (y = 0; y < height; y++) {
        gx_image_plane_t planes;
        int rows_used;

        blend_row(buf_ptr, buf->planestride, width, num_comp, bg, linebuf);
        planes.data = linebuf;
        planes.data_x = 0;
        planes.raster = width * num_comp;
        info->procs->plane_data(info, &planes, 1, &rows_used);
        /* todo: check return value */
        buf_ptr += buf->rowstride;
    }
    gs_free_object(pdev->memory, linebuf_unaligned, "pdf14_put_image");
    info->procs->end_image(info, true);
    /* This will also decrement the device profile */
    rc_decrement_only_cs(pcs, "pdf14_put_image");
    return code;
}