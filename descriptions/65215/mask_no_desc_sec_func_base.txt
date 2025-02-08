GF_EXPORT
void gf_url_to_fs_path(char *sURL)
{
	if (!strnicmp(sURL, "file://", 7)) {
		/*file:///C:\ scheme*/
		// <MASK>
	}

	while (1) {
		char *sep = strstr(sURL, "%20");
		if (!sep) break;
		sep[0] = ' ';
		memmove(sep+1, sep+3, strlen(sep)-2);
	}
}