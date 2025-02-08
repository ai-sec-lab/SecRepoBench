s32 i, End, Total;
	LineBuffer[0] = 0;
	if (Start >= Size) return -1;

	offset = 2;
	End = gf_token_find(SourceBuffer, Start, Size, "\r\n");
	if (End<0) {
		End = gf_token_find(SourceBuffer, Start, Size, "\r");
		if (End<0) End = gf_token_find(SourceBuffer, Start, Size, "\n");
		if (End < 0) return -1;
		offset = 1;
	}

	Total = End - Start + offset;
	if ((u32) Total >= LineBufferSize) Total = LineBufferSize;
	for (i=0; i<Total; i++) LineBuffer[i] = SourceBuffer[Start+i];
	LineBuffer[i] = 0;