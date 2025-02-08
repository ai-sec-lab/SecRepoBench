{
                SkRect devBounds;
                dev->ctm().mapRect(&devBounds, *bounds);
                if (devBounds.intersect(SkRect::Make(clipRect))) {
                    fSrcBounds = devBounds.roundOut();
                    // Check again, now that we have computed srcbounds.
                    fNeedsTiling = fSrcBounds.right() > kMaxDim || fSrcBounds.bottom() > kMaxDim;
                } else {
                    fNeedsTiling = false;
                    fDone = true;
                }
            } else {
                fSrcBounds = clipRect;
            }