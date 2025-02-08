char szLine[2048];
	char *sOK;
	u32 len;
	GF_Err e;
	char *prevLine = NULL;
	char *header = NULL;
	u32 header_len = 0;
	Bool had_marks = GF_FALSE;

	if (!parser) return GF_BAD_PARAM;
	parser->suspend = GF_FALSE;

	if (parser->is_srt) {
		parser->on_header_parsed(parser->user, "WEBVTT\n");
	}
