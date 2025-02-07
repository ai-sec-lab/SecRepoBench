static inline void MPV_motion_lowres(MpegEncContext *s,
                                     uint8_t *dest_y, uint8_t *dest_cb,
                                     uint8_t *dest_cr,
                                     int dir, uint8_t *const *ref_picture,
                                     const h264_chroma_mc_func *pix_op)
{
    int mx, my;
    int mb_x, macroblocky;
    const int lowres  = s->avctx->lowres;
    const int block_s = 8 >>lowres;

    mb_x = s->mb_x;
    macroblocky = s->mb_y;

    switch (s->mv_type) {
    case MV_TYPE_16X16:
        mpeg_motion_lowres(s, dest_y, dest_cb, dest_cr,
                           0, 0, 0,
                           ref_picture, pix_op,
                           s->mv[dir][0][0], s->mv[dir][0][1],
                           2 * block_s, macroblocky);
        break;
    case MV_TYPE_8X8:
        mx = 0;
        my = 0;
        for (int i = 0; i < 4; i++) {
            hpel_motion_lowres(s, dest_y + ((i & 1) + (i >> 1) *
                               s->linesize) * block_s,
                               ref_picture[0], 0, 0,
                               (2 * mb_x + (i & 1)) * block_s,
                               (2 * macroblocky + (i >> 1)) * block_s,
                               s->width, s->height, s->linesize,
                               s->h_edge_pos >> lowres, s->v_edge_pos >> lowres,
                               block_s, block_s, pix_op,
                               s->mv[dir][i][0], s->mv[dir][i][1]);

            mx += s->mv[dir][i][0];
            my += s->mv[dir][i][1];
        }

        if (!CONFIG_GRAY || !(s->avctx->flags & AV_CODEC_FLAG_GRAY))
            chroma_4mv_motion_lowres(s, dest_cb, dest_cr, ref_picture,
                                     pix_op, mx, my);
        break;
    // Handle different motion vector types for low-resolution motion compensation
    // in the context of MPEG video decoding. The function processes macroblocks
    // based on their motion vector type and updates the destination buffers 
    // (luma and chroma) accordingly. The cases handled include field motion vectors
    // (MV_TYPE_FIELD), 16x8 motion vectors (MV_TYPE_16X8), and dual-prime motion 
    // vectors (MV_TYPE_DMV). The function performs separate operations based on 
    // whether the picture is a frame or field picture, and adapts the reference 
    // picture selection accordingly. Additionally, for DMV, it adjusts the 
    // prediction by averaging with the opposite field. This setup allows efficient 
    // handling of various motion compensation scenarios in low-resolution settings.
    // <MASK>
    default:
        av_assert2(0);
    }
}