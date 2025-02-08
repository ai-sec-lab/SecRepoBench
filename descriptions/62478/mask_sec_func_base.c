static int
vips__archive_mkfile_file(VipsArchive *archive,
	const char *filename, void *buf, size_t len)
{
	char *path;
	// <MASK>

	if (vips__file_write(buf, sizeof(char), len, f)) {
		g_free(path);
		fclose(f);
		return -1;
	}

	fclose(f);
	g_free(path);

	return 0;
}