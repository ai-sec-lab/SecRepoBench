static int
vips__archive_mkfile_file(VipsArchive *archive,
	const char *filepath, void *buf, size_t len)
{
	char *path;
	FILE *f;

	path = g_build_filename(archive->base_dirname, filepath, NULL);

	if (!(f = vips__file_open_write(path, FALSE))) {
		g_free(path);
		return -1;
	}

	if (vips__file_write(buf, sizeof(char), len, f)) {
		g_free(path);
		fclose(f);
		return -1;
	}

	fclose(f);
	g_free(path);

	return 0;
}