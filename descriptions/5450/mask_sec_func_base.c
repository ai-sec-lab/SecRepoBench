MagickBooleanType sixel_decode(unsigned char              /* in */  *p,         /* sixel bytes */
                               unsigned char              /* out */ **pixels,   /* decoded pixels */
                               size_t                     /* out */ *pwidth,    /* image width */
                               size_t                     /* out */ *pheight,   /* image height */
                               unsigned char              /* out */ **palette,  /* ARGB palette */
                               size_t                     /* out */ *ncolors    /* palette size (<= 256) */)
{
    int n, i, r, g, b, sixel_vertical_mask, c;
    int posision_x, posision_y;
    int max_x, max_y;
    int attributed_pan, attributed_pad;
    int attributed_ph, attributed_pv;
    int repeat_count, color_index, max_color_index = 2, background_color_index;
    int param[10];
    int sixel_palet[SIXEL_PALETTE_MAX];
    unsigned char *imbuf, *dmbuf;
    int imsx, imsy;
    int dmsx, dmsy;
    int y;
    // <MASK>

    if (++max_x < attributed_ph) {
        max_x = attributed_ph;
    }
    if (++max_y < attributed_pv) {
        max_y = attributed_pv;
    }

    if (imsx > max_x || imsy > max_y) {
        dmsx = max_x;
        dmsy = max_y;
        if ((dmbuf = (unsigned char *) AcquireQuantumMemory(dmsx , dmsy)) == NULL) {
            imbuf = (unsigned char *) RelinquishMagickMemory(imbuf);
            return (MagickFalse);
        }
        for (y = 0; y < dmsy; ++y) {
            (void) CopyMagickMemory(dmbuf + dmsx * y, imbuf + imsx * y, dmsx);
        }
        imbuf = (unsigned char *) RelinquishMagickMemory(imbuf);
        imsx = dmsx;
        imsy = dmsy;
        imbuf = dmbuf;
    }

    *pixels = imbuf;
    *pwidth = imsx;
    *pheight = imsy;
    *ncolors = max_color_index + 1;
    *palette = (unsigned char *) AcquireQuantumMemory(*ncolors,4);
    if (*palette == (unsigned char *) NULL)
      return(MagickFalse);
    for (n = 0; n < (ssize_t) *ncolors; ++n) {
        (*palette)[n * 4 + 0] = sixel_palet[n] >> 16 & 0xff;
        (*palette)[n * 4 + 1] = sixel_palet[n] >> 8 & 0xff;
        (*palette)[n * 4 + 2] = sixel_palet[n] & 0xff;
        (*palette)[n * 4 + 3] = 0xff;
    }
    return(MagickTrue);
}