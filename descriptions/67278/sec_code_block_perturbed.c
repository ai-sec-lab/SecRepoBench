u32 nb_skipped = 0;
		if (!latm_dmx_sync_frame_bs(bs, &acfg, 0, NULL, &nb_skipped)) break;
		if (acfg.base_sr_index >= GF_ARRAY_LENGTH(GF_M4ASampleRates) || GF_M4ASampleRates[acfg.base_sr_index] == 0) {
			numframes = 0;
			break;
		}

		if (nb_skipped) {
			if (nb_skip) {
				numframes = 0;
				break;
			}
			nb_skip++;
		}
		numframes++;