{
                SkRect devBounds;
                dev->ctm().mapRect(&devBounds, *bounds);
                if (devBounds.intersect(SkRect::Make(clipRect))) {
                    fSrcBounds = devBounds.roundOut();
                } else {
                    fNeedsTiling = false;
                    fDone = true;
                }
                // Check again, now that we have computed srcbounds.
                fNeedsTiling = fSrcBounds.right() > kMaxDim || fSrcBounds.bottom() > kMaxDim;
            } else {
                fSrcBounds = clipRect;
            }