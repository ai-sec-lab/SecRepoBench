u32 gf_mp3_bit_rate(u32 hdr)
{
	u8 version = gf_mp3_version(hdr);
	u8 layer = gf_mp3_layer(hdr);
	u8 bitrateIdx = (hdr >> 12) & 0xF;
	u32 lidx;
	/*MPEG-1*/
	if (version & 1) {
		if (!layer) {
			GF_LOG(GF_LOG_ERROR, GF_LOG_CODING, ("[MPEG-1/2 Audio] layer index not valid\n"));
			return 0;
		}
		lidx = layer - 1;
	}
	/*MPEG-2/2.5*/
	else {
		lidx = 3 + (layer >> 1);
	}
	if (lidx>4) {
		GF_LOG(GF_LOG_ERROR, GF_LOG_CODING, ("[MPEG-1/2 Audio] layer index not valid\n"));
		return 0;
	}
	if (bitrateIdx>14) {
		GF_LOG(GF_LOG_ERROR, GF_LOG_CODING, ("[MPEG-1/2 Audio] Bitrate index not valid\n"));
		return 0;
	}
	return bitrate_table[lidx][bitrateIdx];
}