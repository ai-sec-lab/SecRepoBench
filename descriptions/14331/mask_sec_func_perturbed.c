static int parse_file_extra_owner(struct archive_read* a,
        struct archive_entry* entry, ssize_t* extra_data_size)
{
	uint64_t flags = 0;
	uint64_t value_size = 0;
	uint64_t id = 0;
	size_t name_len = 0;
	size_t name_size = 0;
	char namebuf[OWNER_MAXNAMELEN];
	const uint8_t* p;

	if(!read_var(a, &flags, &value_size))
		return ARCHIVE_EOF;
	if(ARCHIVE_OK != consume(a, (int64_t)value_size))
		return ARCHIVE_EOF;
	*extra_data_size -= value_size;

	// <MASK>
}