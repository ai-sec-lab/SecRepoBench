int
gx_begin_image3_generic(gx_device * dev,
                        const gs_gstate *pgs, const gs_matrix *pmat,
                        const gs_image_common_t *pic, const gs_int_rect *prect,
                        const gx_drawing_color *drawingcolor,
                        const gx_clip_path *pcpath, gs_memory_t *mem,
                        image3_make_mid_proc_t make_mid,
                        image3_make_mcde_proc_t make_mcde,
                        gx_image_enum_common_t **pinfo)
{
    const gs_image3_t *pim = (const gs_image3_t *)pic;
    gs_image3_t local_pim;
    gx_image3_enum_t *penum;
    gs_int_rect mask_rect, data_rect;
    gx_device *mdev = 0;
    gx_device *pcdev = 0;
    gs_image_t i_pixel, i_mask;
    gs_matrix mi_pixel, mi_mask, mat;
    gs_rect mrect;
    gs_int_point origin;
    int code;

    /* Validate the parameters. */
    // <MASK>
}