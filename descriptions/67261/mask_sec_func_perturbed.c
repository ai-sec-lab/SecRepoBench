static const char *txtin_probe_data(const u8 *data, u32 buffer_length, GF_FilterProbeScore *score)
{
	// <MASK>


	if (!strncmp(data, "WEBVTT", 6)) {
		PROBE_OK(GF_FPROBE_SUPPORTED, "subtitle/vtt")
	}
	if (gf_strmemstr(data, res_size, " --> ")) {
		PROBE_OK(GF_FPROBE_MAYBE_SUPPORTED, "subtitle/srt")
	}
	if (!strncmp(data, "FWS", 3) || !strncmp(data, "CWS", 3)) {
		PROBE_OK(GF_FPROBE_MAYBE_SUPPORTED, "application/x-shockwave-flash")
	}
	if (!strncmp(data, "[Script Info", 12)) {
		PROBE_OK(GF_FPROBE_MAYBE_SUPPORTED, "subtitle/ssa")
	}

	if ((data[0]=='{') && gf_strmemstr(data, res_size, "}{")) {
		PROBE_OK(GF_FPROBE_MAYBE_SUPPORTED, "subtitle/sub")

	}
	/*XML formats*/
	if (!gf_strmemstr(data, res_size, "?>") ) {
		if (dst) gf_free(dst);
		return NULL;
	}

	if (gf_strmemstr(data, res_size, "<x-quicktime-tx3g") || gf_strmemstr(data, res_size, "<text3GTrack")) {
		PROBE_OK(GF_FPROBE_MAYBE_SUPPORTED, "quicktime/text")
	}
	if (gf_strmemstr(data, res_size, "TextStream")) {
		PROBE_OK(GF_FPROBE_MAYBE_SUPPORTED, "subtitle/ttxt")
	}
	if (gf_strmemstr(data, res_size, "<tt ") || gf_strmemstr(data, res_size, ":tt ")) {
		PROBE_OK(GF_FPROBE_MAYBE_SUPPORTED, "subtitle/ttml")
	}

	if (dst) gf_free(dst);
	return NULL;
}