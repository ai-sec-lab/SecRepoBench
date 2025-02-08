s32 End, Total;
	LineBuffer[0] = 0;
	if (Start >= Size) return -1;

	offset = 2;
	End = gf_token_find(Buffer, Start, Size, "\r\n");
	if (End<0) {
		End = gf_token_find(Buffer, Start, Size, "\r");
		if (End<0) End = gf_token_find(Buffer, Start, Size, "\n");
		if (End < 0) return -1;
		offset = 1;
	}

	Total = End - Start + offset;
	if ((u32) Total >= LineBufferSize) Total = LineBufferSize-1;
	memcpy(LineBuffer, Buffer + Start, Total);
	LineBuffer[Total] = 0;