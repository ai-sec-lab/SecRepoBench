static const char *txtin_probe_data(const u8 *buffer, u32 data_size, GF_FilterProbeScore *score)
{
	char *dst = NULL;
	char *res=NULL;
	u32 res_size=0;

	GF_Err e = gf_utf_get_string_from_bom((char *)buffer, data_size, &dst, &res, &res_size);
	if (e) return NULL;

	buffer = res;
	//strip all spaces and \r\n\t
	while (buffer[0] && strchr("\n\r\t ", (char) buffer[0])) {
		buffer++;
		res_size--;
	}

#define PROBE_OK(_score, _mime) \
		*score = _score;\
		if (dst) gf_free(dst);\
		return _mime; \


	if (!strncmp(buffer, "WEBVTT", 6)) {
		PROBE_OK(GF_FPROBE_SUPPORTED, "subtitle/vtt")
	}
	if (gf_strmemstr(buffer, res_size, " --> ")) {
		PROBE_OK(GF_FPROBE_MAYBE_SUPPORTED, "subtitle/srt")
	}
	if (!strncmp(buffer, "FWS", 3) || !strncmp(buffer, "CWS", 3)) {
		PROBE_OK(GF_FPROBE_MAYBE_SUPPORTED, "application/x-shockwave-flash")
	}
	if (!strncmp(buffer, "[Script Info", 12)) {
		PROBE_OK(GF_FPROBE_MAYBE_SUPPORTED, "subtitle/ssa")
	}

	if ((buffer[0]=='{') && gf_strmemstr(buffer, res_size, "}{")) {
		PROBE_OK(GF_FPROBE_MAYBE_SUPPORTED, "subtitle/sub")

	}
	/*XML formats*/
	if (!gf_strmemstr(buffer, res_size, "?>") ) {
		if (dst) gf_free(dst);
		return NULL;
	}

	if (gf_strmemstr(buffer, res_size, "<x-quicktime-tx3g") || gf_strmemstr(buffer, res_size, "<text3GTrack")) {
		PROBE_OK(GF_FPROBE_MAYBE_SUPPORTED, "quicktime/text")
	}
	if (gf_strmemstr(buffer, res_size, "TextStream")) {
		PROBE_OK(GF_FPROBE_MAYBE_SUPPORTED, "subtitle/ttxt")
	}
	if (gf_strmemstr(buffer, res_size, "<tt ") || gf_strmemstr(buffer, res_size, ":tt ")) {
		PROBE_OK(GF_FPROBE_MAYBE_SUPPORTED, "subtitle/ttml")
	}

	if (dst) gf_free(dst);
	return NULL;
}