FILE *f;

	path = g_build_filename(archive->base_dirname, filepath, NULL);

	if (!(f = vips__file_open_write(path, FALSE))) {
		g_free(path);
		return -1;
	}