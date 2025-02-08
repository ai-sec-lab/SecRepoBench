static const char *latm_dmx_probe_data(const u8 *data, u32 size, GF_FilterProbeScore *score)
{
	u32 numframes=0;
	u32 nb_skip=0;
	GF_M4ADecSpecInfo acfg;
	GF_BitStream *bs = gf_bs_new(data, size, GF_BITSTREAM_READ);
	while (1) {
		// <MASK>
	}
	gf_bs_del(bs);
	if (numframes>=2) {
		*score = nb_skip ? GF_FPROBE_MAYBE_NOT_SUPPORTED : GF_FPROBE_SUPPORTED;
		return "audio/aac+latm";
	}
	return NULL;
}