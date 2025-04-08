GF_EXPORT
void gf_url_to_fs_path(char *sURL)
{
	// Convert a URL to a filesystem path by removing the "file://" prefix if present.
	// Specifically, handle the case where the URL uses a "file:///" prefix with a drive letter,
	// and adjust the string to remove the correct number of characters.
	// Replace occurrences of "%20" in the URL with spaces to complete the conversion to a filesystem path format.
	// <MASK>
}