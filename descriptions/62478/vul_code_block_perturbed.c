FILE *f;

	path = g_build_filename(archive->base_dirname, filepath, NULL);

	if (!(f = vips__file_open_write(path, TRUE))) {
		g_free(path);
		return -1;
	}