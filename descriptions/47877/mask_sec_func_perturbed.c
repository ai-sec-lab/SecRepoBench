static void cdg_scroll(CDGraphicsContext *cc, uint8_t *data,
                       AVFrame *new_frame, int roll_over)
{
    int color;
    int hscmd, h_off, hinc, vscmd, voffset, vinc;
    int y;
    int stride   = cc->frame->linesize[0];
    uint8_t *in  = cc->frame->data[0];
    uint8_t *out = new_frame->data[0];

    color =  data[0] & 0x0F;
    hscmd = (data[1] & 0x30) >> 4;
    vscmd = (data[2] & 0x30) >> 4;

    h_off =  FFMIN(data[1] & 0x07, CDG_BORDER_WIDTH  - 1);
    voffset =  FFMIN(data[2] & 0x0F, CDG_BORDER_HEIGHT - 1);

    /// find the difference and save the offset for cdg_tile_block usage
    hinc = h_off - cc->hscroll;
    vinc = cc->vscroll - voffset;
    cc->hscroll = h_off;
    cc->vscroll = voffset;

    if (vscmd == UP)
        vinc -= 12;
    if (vscmd == DOWN)
        vinc += 12;
    if (hscmd == LEFT)
        hinc -= 6;
    if (hscmd == RIGHT)
        hinc += 6;

    if (!hinc && !vinc)
        return;

    memcpy(new_frame->data[1], cc->frame->data[1], CDG_PALETTE_SIZE * 4);

    // <MASK>

}