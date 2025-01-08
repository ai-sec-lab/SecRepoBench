if (acfg.base_sr_index >= GF_ARRAY_LENGTH(GF_M4ASampleRates) || GF_M4ASampleRates[acfg.base_sr_index] == 0) {
			nb_frames = 0;
			break;
		}