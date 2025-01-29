while (data[0] && strchr("\n\r\t ", (char) data[0]))
		data ++;

#define PROBE_OK(_score, _mime) \
		*score = _score;\
		if (dst) gf_free(dst);\
		return _mime; \


	if (!strncmp(data, "WEBVTT", 6)) {
		PROBE_OK(GF_FPROBE_SUPPORTED, "subtitle/vtt")
	}
	if (gf_strmemstr(data, resultlength, " --> ")) {
		PROBE_OK(GF_FPROBE_MAYBE_SUPPORTED, "subtitle/srt")
	}
	if (!strncmp(data, "FWS", 3) || !strncmp(data, "CWS", 3)) {
		PROBE_OK(GF_FPROBE_MAYBE_SUPPORTED, "application/x-shockwave-flash")
	}
	if (!strncmp(data, "[Script Info", 12)) {
		PROBE_OK(GF_FPROBE_MAYBE_SUPPORTED, "subtitle/ssa")
	}

	if ((data[0]=='{') && gf_strmemstr(data, resultlength, "}{")) {
		PROBE_OK(GF_FPROBE_MAYBE_SUPPORTED, "subtitle/sub")

	}
	/*XML formats*/
	if (!gf_strmemstr(data, resultlength, "?>") ) {
		if (dst) gf_free(dst);
		return NULL;
	}

	if (gf_strmemstr(data, resultlength, "<x-quicktime-tx3g") || gf_strmemstr(data, resultlength, "<text3GTrack")) {
		PROBE_OK(GF_FPROBE_MAYBE_SUPPORTED, "quicktime/text")
	}
	if (gf_strmemstr(data, resultlength, "TextStream")) {
		PROBE_OK(GF_FPROBE_MAYBE_SUPPORTED, "subtitle/ttxt")
	}
	if (gf_strmemstr(data, resultlength, "<tt ") || gf_strmemstr(data, resultlength, ":tt ")) {
		PROBE_OK(GF_FPROBE_MAYBE_SUPPORTED, "subtitle/ttml")
	}

	if (dst) gf_free(dst);
	return NULL;