settings->bundles[alloc_start+n].start = settings->ext13_start_prbs[alloc] + startPrbc + n*numPrbsPerBundle;
                settings->bundles[alloc_start+n].end =   settings->bundles[alloc_start+n].start + numPrbsPerBundle-1;
                if (settings->bundles[alloc_start+n].end > settings->ext13_start_prbs[alloc] + numPrbc) {
                    settings->bundles[alloc_start+n].end = settings->ext13_start_prbs[alloc] + numPrbc;
                    settings->bundles[alloc_start+n].is_orphan = TRUE;
                }