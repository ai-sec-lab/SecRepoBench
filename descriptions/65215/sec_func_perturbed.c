GF_EXPORT
void gf_url_to_fs_path(char *urlString)
{
	if (!strnicmp(urlString, "file://", 7)) {
		/*file:///C:\ scheme*/
		if ((strlen(urlString)>=10) && (urlString[7]=='/') && (urlString[9]==':')) {
			memmove(urlString, urlString+8, strlen(urlString)-7);
		} else {
			memmove(urlString, urlString+7, strlen(urlString)-6);
		}
	}

	while (1) {
		char *sep = strstr(urlString, "%20");
		if (!sep) break;
		sep[0] = ' ';
		memmove(sep+1, sep+3, strlen(sep)-2);
	}
}