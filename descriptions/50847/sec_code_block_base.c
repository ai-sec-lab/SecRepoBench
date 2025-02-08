if (e->arg1 < 0 || e->arg1 > pts->n_points
                 || (gOutline->pointCount + e->arg2) < 0 || (gOutline->pointCount + e->arg2) > pts->n_points) {
                    error = fBadFontData;
                    goto ex;
                }
                else {
                    e->m.tx = (pts->org_x[e->arg1] - pts->org_x[gOutline->pointCount + e->arg2]) << 10;
                    e->m.ty = (pts->org_y[e->arg1] - pts->org_y[gOutline->pointCount + e->arg2]) << 10;
                }