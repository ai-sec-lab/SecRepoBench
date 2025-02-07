char *gf_text_get_utf8_line(char *szLine, u32 lineSize, FILE *txt_in, s32 unicode_type)
{
	u32 i, j, len;
	char *sOK;
	char szLineConv[2048];
	unsigned short *sptr;

	memset(szLine, 0, sizeof(char)*lineSize);
	sOK = gf_fgets(szLine, lineSize, txt_in);
	if (!sOK) return NULL;
	if (unicode_type<=1) {
		j=0;
		len = (u32) strlen(szLine);
		for (i=0; i<len; i++) {
			if (!unicode_type && (szLine[i] & 0x80)) {
				/*non UTF8 (likely some win-CP)*/
				if ((szLine[i+1] & 0xc0) != 0x80) {
					if (j >= GF_ARRAY_LENGTH(szLineConv))
						break;
					szLineConv[j] = 0xc0 | ( (szLine[i] >> 6) & 0x3 );
					j++;
					szLine[i] &= 0xbf;
				}
				/*UTF8 2 bytes char*/
				else if ( (szLine[i] & 0xe0) == 0xc0) {
					if (j >= GF_ARRAY_LENGTH(szLineConv))
						break;
					szLineConv[j] = szLine[i];
					i++;
					j++;
				}
				/*UTF8 3 bytes char*/
				else if ( (szLine[i] & 0xf0) == 0xe0) 
				// This code block handles the conversion of multi-byte UTF-8 characters to a custom internal format.
				// It processes characters from the input string `szLine` and writes them into `szLineConv` while
				// ensuring the proper handling of 2-byte, 3-byte, and 4-byte UTF-8 characters.
				// For each UTF-8 character, it checks the first byte to determine the number of bytes in the character,
				// then copies the appropriate number of bytes to `szLineConv`.
				// <MASK>
			}

			if (j >= GF_ARRAY_LENGTH(szLineConv))
				break;

			szLineConv[j] = szLine[i];
			j++;


		}
		if ( j >= GF_ARRAY_LENGTH(szLineConv) ) {
			GF_LOG(GF_LOG_DEBUG, GF_LOG_PARSER, ("[TXTIn] Line too long to convert to utf8 (len: %d)\n", len));
			j = GF_ARRAY_LENGTH(szLineConv) -1 ;
		}
		if ( j >= lineSize ) {
			GF_LOG(GF_LOG_DEBUG, GF_LOG_PARSER, ("[TXTIn] UT8 converted line too long for buffer (len: %d, buffer: %d)\n", j, lineSize));
			j = lineSize-1 ;
		}
		szLineConv[j] = 0;
		strcpy(szLine, szLineConv);
		return sOK;
	}

#ifdef GPAC_BIG_ENDIAN
	if (unicode_type==3)
#else
	if (unicode_type==2)
#endif
	{
		i=0;
		while (1) {
			char c;
			if (!szLine[i] && !szLine[i+1]) break;
			c = szLine[i+1];
			szLine[i+1] = szLine[i];
			szLine[i] = c;
			i+=2;
		}
	}
	sptr = (u16 *)szLine;
	i = gf_utf8_wcstombs(szLineConv, 2048, (const unsigned short **) &sptr);
	if (i == GF_UTF8_FAIL) i = 0;
	szLineConv[i] = 0;
	strcpy(szLine, szLineConv);
	/*this is ugly indeed: since input is UTF16-LE, there are many chances the gf_fgets never reads the \0 after a \n*/
	if (unicode_type==3) gf_fgetc(txt_in);
	return sOK;
}