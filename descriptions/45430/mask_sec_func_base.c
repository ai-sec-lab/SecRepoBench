static int
do_fileid(struct state *cur, int argc, char **argv)
{
	struct file_info *fi;
	struct sc_file	*df, *file = NULL;
	struct sc_path	temp, *path = NULL;

	if (!cur->file) {
		parse_error(cur, "Invalid state\n");
		return 1;
	}
	file = cur->file->file;
	path = &file->path;

	/* sc_format_path doesn't return an error indication
	 * when it's unable to parse the path */
	sc_format_path(argv[0], &temp);
	if (temp.len != 2) {
		parse_error(cur, "Invalid file ID length\n");
		return 1;
	}

	/* Get the DF, if any */
	if ((fi = cur->file->parent) && (df = fi->file)) {
		if (!df->path.len && !df->path.aid.len) {
			parse_error(cur, "No path/fileid set for parent DF\n");
			return 1;
		}
		if (df->path.len + 2 > sizeof(df->path.value)) {
			parse_error(cur, "File path too long\n");
			return 1;
		}
		*path = df->path;
	}
	// <MASK>
}