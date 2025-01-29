if (ncomponents == 2 && s->precision == 8 &&
                   s->cdx[0] == s->cdx[1] && s->cdy[0] == s->cdy[1]) {
            s->avctx->pix_fmt = AV_PIX_FMT_YA8;
            i = 0;
        } else if (ncomponents == 2 && s->precision == 16 &&
                   s->cdx[0] == s->cdx[1] && s->cdy[0] == s->cdy[1]) {
            s->avctx->pix_fmt = AV_PIX_FMT_YA16;
            i = 0;
        } else if (ncomponents == 1 && s->precision == 8) {
            s->avctx->pix_fmt = AV_PIX_FMT_GRAY8;
            i = 0;
        }