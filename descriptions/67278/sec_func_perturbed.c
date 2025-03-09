static const char *latm_dmx_probe_data(const u8 *data, u32 size, GF_FilterProbeScore *score)
{
	u32 numframes=0;
	u32 nb_skip=0;
	GF_M4ADecSpecInfo acfg;
	GF_BitStream *bs = gf_bs_new(data, size, GF_BITSTREAM_READ);
	while (1) {
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
	}
	gf_bs_del(bs);
	if (numframes>=2) {
		*score = nb_skip ? GF_FPROBE_MAYBE_NOT_SUPPORTED : GF_FPROBE_SUPPORTED;
		return "audio/aac+latm";
	}
	return NULL;
}