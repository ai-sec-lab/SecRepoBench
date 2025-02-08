static void ext11_work_out_bundles(guint startPrbc,
                                   guint numPrbc,
                                   guint numPrbsPerBundle,             /* number of PRBs pre (full) bundle */
                                   ext11_settings_t *settings)
{
    /* Allocation configured by ext 6 */
    if (settings->ext6_set) {
        guint bundles_per_entry = settings->ext6_rbg_size / numPrbsPerBundle;

        guint bundles_set = 0;
        for (guint8 n=0; n < settings->ext6_num_bits_set; n++) {
            /* For each bit set in the mask */
            guint32 prb_start = settings->ext6_bits_set[n] * settings->ext6_rbg_size;

            /* For each bundle within identified rbgSize block */
            for (guint m=0; m < bundles_per_entry; m++) {
                settings->bundles[bundles_set].start = prb_start+(m*numPrbsPerBundle);
                /* Start already beyond end, so doesn't count. */
                if (settings->bundles[bundles_set].start > (startPrbc+numPrbc)) {
                    break;
                }
                settings->bundles[bundles_set].end = prb_start+((m+1)*numPrbsPerBundle)-1;
                if (settings->bundles[bundles_set].end > numPrbc) {
                    /* Extends beyond end, so counts but is an orphan bundle */
                    settings->bundles[bundles_set].end = numPrbc;
                    settings->bundles[bundles_set].is_orphan = TRUE;
                }
                bundles_set++;
                if (bundles_set == MAX_BFW_BUNDLES) {
                    return;
                }
            }
        }
        settings->num_bundles = bundles_set;
    }

    /* Allocation configured by ext 12 */
    else if (settings->ext12_set) {
        /* First, allocate normally from startPrbc, numPrbc */
        settings->num_bundles = (numPrbc+numPrbsPerBundle-1) / numPrbsPerBundle;

        /* Don't overflow settings->bundles[] ! */
        settings->num_bundles = MIN(MAX_BFW_BUNDLES, settings->num_bundles);

        for (guint32 n=0; n < settings->num_bundles; n++) {
            settings->bundles[n].start = startPrbc + n*numPrbsPerBundle;
            settings->bundles[n].end =   settings->bundles[n].start + numPrbsPerBundle-1;
            /* Does it go beyond the end? */
            if (settings->bundles[n].end > startPrbc+numPrbc) {
                settings->bundles[n].end = numPrbc+numPrbc;
                settings->bundles[n].is_orphan = TRUE;
            }
        }
        if (settings->num_bundles == MAX_BFW_BUNDLES) {
            return;
        }

        guint prb_offset = startPrbc + numPrbc;

        /* Loop over pairs, adding bundles for each */
        for (guint p=0; p < settings->ext12_num_pairs; p++) {
            prb_offset += settings->ext12_pairs[p].off_start_prb;
            guint pair_bundles = (settings->ext12_pairs[p].num_prb+numPrbsPerBundle-1) / numPrbsPerBundle;

            for (guint32 n=0; n < pair_bundles; n++) {
                guint idx = settings->num_bundles;

                settings->bundles[idx].start = prb_offset + n*numPrbsPerBundle;
                settings->bundles[idx].end =   settings->bundles[idx].start + numPrbsPerBundle-1;
                /* Does it go beyond the end? */
                if (settings->bundles[idx].end > prb_offset + settings->ext12_pairs[p].num_prb) {
                    settings->bundles[idx].end = prb_offset + settings->ext12_pairs[p].num_prb;
                    settings->bundles[idx].is_orphan = TRUE;
                }
                /* Range check / return */
                settings->num_bundles++;
                if (settings->num_bundles == MAX_BFW_BUNDLES) {
                    return;
                }
            }

            prb_offset += settings->ext12_pairs[p].num_prb;
        }
    }

    /* Allocation configured by ext 13 */
    else if (settings->ext13_set) {
        guint alloc_size = (numPrbc+numPrbsPerBundle-1) / numPrbsPerBundle;
        settings->num_bundles = alloc_size * settings->ext13_num_start_prbs;

        /* Don't overflow settings->bundles[] ! */
        settings->num_bundles = MIN(MAX_BFW_BUNDLES, settings->num_bundles);

        for (guint alloc=0; alloc < settings->ext13_num_start_prbs; alloc++) {
            guint alloc_start = alloc * alloc_size;
            for (guint32 n=0; n < alloc_size; n++) {
                // <MASK>
            }
        }
    }

    /* Bundles not controlled by other extensions - just divide up range into bundles we have */
    else {
        settings->num_bundles = (numPrbc+numPrbsPerBundle-1) / numPrbsPerBundle;

        /* Don't overflow settings->bundles[] ! */
        settings->num_bundles = MIN(MAX_BFW_BUNDLES, settings->num_bundles);

        for (guint32 n=0; n < settings->num_bundles; n++) {
            settings->bundles[n].start = startPrbc + n*numPrbsPerBundle;
            settings->bundles[n].end =   settings->bundles[n].start + numPrbsPerBundle-1;
            /* Does it go beyond the end? */
            if (settings->bundles[n].end > startPrbc+numPrbc) {
                settings->bundles[n].end = numPrbc+numPrbc;
                settings->bundles[n].is_orphan = TRUE;
            }
        }
    }
}