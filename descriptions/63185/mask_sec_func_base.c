int
gx_image_enum_begin(gx_device * dev, const gs_gstate * pgs,
                    const gs_matrix *pmat, const gs_image_common_t * pic,
                const gx_drawing_color * pdcolor, const gx_clip_path * pcpath,
                gs_memory_t * mem, gx_image_enum *penum)
{
    const gs_pixel_image_t *pim = (const gs_pixel_image_t *)pic;
    gs_image_format_t format = pim->format;
    const int width = pim->Width;
    const int height = pim->Height;
    const int bps = pim->BitsPerComponent;
    bool masked = penum->masked;
    const float *decode = pim->Decode;
    gs_matrix_double mat;
    int index_bps;
    gs_color_space *pcs = pim->ColorSpace;
    gs_logical_operation_t lop = (pgs ? pgs->log_op : lop_default);
    int code;
    int log2_xbytes = (bps <= 8 ? 0 : arch_log2_sizeof_frac);
    int spp, nplanes, spread;
    uint bsize;
    byte *buffer = NULL;
    fixed mtx, mty;
    gs_fixed_point row_extent, col_extent, x_extent, y_extent;
    bool device_color = true;
    gs_fixed_rect obox, cbox;
    bool gridfitimages = 0;
    bool in_pattern_accumulator;
    bool in_smask;
    int orthogonal;
    int force_interpolation = 0;

    // <MASK>
    penum->pcs = NULL;
    gs_free_object(mem, penum, "gx_begin_image1");
    return code;
}