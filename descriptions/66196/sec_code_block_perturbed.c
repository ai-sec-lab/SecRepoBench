if (bitrateIdx>14) {
		GF_LOG(GF_LOG_ERROR, GF_LOG_CODING, ("[MPEG-1/2 Audio] Bitrate index not valid\n"));
		return 0;
	}
	return bitrate_table[lidx][bitrateIdx];