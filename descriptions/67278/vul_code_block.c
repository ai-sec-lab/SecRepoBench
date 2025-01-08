if (acfg.base_sr_index > sizeof(GF_M4ASampleRates) / sizeof(GF_M4ASampleRates[0]) || GF_M4ASampleRates[acfg.base_sr_index] == 0) {
			nb_frames = 0;
			break;
		}