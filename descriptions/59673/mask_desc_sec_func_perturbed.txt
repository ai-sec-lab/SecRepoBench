static av_always_inline void mpeg_motion_lowres(MpegEncContext *s,
                                                uint8_t *dest_y,
                                                uint8_t *dest_cb,
                                                uint8_t *dest_cr,
                                                int field_based,
                                                int bottom_field,
                                                int field_select,
                                                uint8_t *const *ref_picture,
                                                const h264_chroma_mc_func *pix_op,
                                                int motion_x, int motion_y,
                                                int h, int mb_y)
{
    const uint8_t *ptr_y, *ptr_cb, *ptr_cr;
    int mx, my, sourcex, src_y, uvsrc_x, uvsrc_y, sx, sy, uvsx, uvsy;
    ptrdiff_t uvlinesize, linesize;
    const int lowres     = s->avctx->lowres;
    const int op_index   = FFMIN(lowres - 1 + s->chroma_x_shift, 3);
    const int block_s    = 8 >> lowres;
    const int s_mask     = (2 << lowres) - 1;
    const int h_edge_pos = s->h_edge_pos >> lowres;
    const int v_edge_pos = s->v_edge_pos >> lowres;
    // This code snippet handles half-pel motion compensation in low-resolution
    // mode for MPEG video decoding. It accounts for motion vectors and adjusts
    // chroma (color) components based on pixel format. The snippet calculates
    // source positions and applies edge emulation if motion goes beyond the
    // frame boundaries. It processes luma and chroma planes separately, 
    // considering field-based encoding and half-pel accuracy. The snippet uses
    // pix_op functions for chroma motion compensation, depending on the lowres
    // level and pixel format (e.g., H263, H261). It includes handling for 
    // specific quirks like quarter-pixel motion and gray mode, ensuring 
    // proper image construction.
    // <MASK>
    // FIXME h261 lowres loop filter
}