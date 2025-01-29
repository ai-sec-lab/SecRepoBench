if (bx >= avctx->width)
                    return AVERROR_INVALIDDATA;

                for (; by < y * 16 + sy2 && by < avctx->height;) {
                    ret = decode_value(s, s->op_model[ptype], 6, 1000, &ptype);
                    if (ret < 0)
                        return ret;
                    if (ptype == 0) {
                        ret = decode_units(s, &r, &g, &b, &cx, &cx1);
                        if (ret < 0)
                            return ret;

                        clr = (b << 16) + (g << 8) + r;
                    }
                    if (ptype > 5)
                        return AVERROR_INVALIDDATA;
                    ret = decode_value(s, s->run_model[ptype], 256, 400, &run);
                    if (ret < 0)
                        return ret;
                    if (run <= 0)
                        return AVERROR_INVALIDDATA;

                    ret = decode_run_p(avctx, ptype, run, x, y, clr,
                                       dst, prev, linesize, plinesize, &bx, &by,
                                       backstep, sx1, sx2, &cx, &cx1);
                    if (ret < 0)
                        return ret;
                }