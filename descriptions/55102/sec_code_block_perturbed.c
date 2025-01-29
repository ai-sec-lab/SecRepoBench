if (bx >= avctx->width)
                    return AVERROR_INVALIDDATA;

                for (; by < y * 16 + sy2 && by < avctx->height;) {
                    ret = decode_value3(s, 5, &s->op_model3[ptype].cntsum,
                                        s->op_model3[ptype].freqs[0],
                                        s->op_model3[ptype].freqs[1],
                                        s->op_model3[ptype].cnts,
                                        s->op_model3[ptype].dectab, &ptype);
                    if (ret < 0)
                        return ret;
                    if (ptype == 0) {
                        ret = decode_units3(s, &r, &g, &b, &cx, &cx1);
                        if (ret < 0)
                            return ret;

                        clr = (b << 16) + (g << 8) + r;
                    }
                    if (ptype > 5)
                        return AVERROR_INVALIDDATA;
                    ret = decode_value3(s, 255, &s->run_model3[ptype].cntsum,
                                        s->run_model3[ptype].freqs[0],
                                        s->run_model3[ptype].freqs[1],
                                        s->run_model3[ptype].cnts,
                                        s->run_model3[ptype].dectab, &length);
                    if (ret < 0)
                        return ret;
                    if (length <= 0)
                        return AVERROR_INVALIDDATA;

                    ret = decode_run_p(avctx, ptype, length, x, y, clr,
                                       dst, prev, linesize, plinesize, &bx, &by,
                                       backstep, sx1, sx2, &cx, &cx1);
                    if (ret < 0)
                        return ret;
                }