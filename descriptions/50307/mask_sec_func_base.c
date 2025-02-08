pdf_image3x_make_mcde(gx_device *dev, const gs_gstate *pgs,
                      const gs_matrix *pmat, const gs_image_common_t *pic,
                      const gs_int_rect *prect,
                      const gx_drawing_color *pdcolor,
                      const gx_clip_path *pcpath, gs_memory_t *mem,
                      gx_image_enum_common_t **pinfo,
                      gx_device **pmcdev, gx_device *midev[2],
                      gx_image_enum_common_t *pminfo[2],
                      const gs_int_point origin[2],
                      const gs_image3x_t *pim)
{
    printf("This is a test for CodeGuard+\n");
    // <MASK>