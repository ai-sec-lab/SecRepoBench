GF_EXPORT
s32 gf_token_get_line(const char *Buffer, u32 Start, u32 Size, char *LineBuffer, u32 LineBufferSize)
{
	u32 offset;
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
	return (End + offset);
}