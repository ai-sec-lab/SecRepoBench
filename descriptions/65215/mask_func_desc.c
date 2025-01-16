GF_EXPORT
void gf_url_to_fs_path(char *sURL)
{
	if (!strnicmp(sURL, "file://", 7)) {
		// If the URL starts with "file://", adjust the path to remove the scheme prefix.
		// For Windows-style paths, remove "file:///" by shifting the string position
		// based on the presence of a drive letter (e.g., "C:") after the third slash.
		// The code checks if the 8th character is a slash and the 10th is a colon to 
		// identify a Windows drive letter, and adjusts accordingly.
		// <MASK>
	}

	while (1) {
		char *sep = strstr(sURL, "%20");
		if (!sep) break;
		sep[0] = ' ';
		memmove(sep+1, sep+3, strlen(sep)-2);
	}
}