if ((flags & OWNER_USER_NAME) != 0) {
		if(!read_var_sized(a, &name_size, NULL))
			return ARCHIVE_EOF;
	        *extra_data_size -= name_size + 1;
		if(!read_ahead(a, name_size, &p))
			return ARCHIVE_EOF;
		if (name_size >= OWNER_MAXNAMELEN)
			name_len = OWNER_MAXNAMELEN - 1;
		else
			name_len = name_size;
		memcpy(namebuf, p, name_len);
		namebuf[name_len] = 0;
		if(ARCHIVE_OK != consume(a, (int64_t)name_size))
			return ARCHIVE_EOF;

		archive_entry_set_uname(entry, namebuf);
	}
	if ((flags & OWNER_GROUP_NAME) != 0) {
		if(!read_var_sized(a, &name_size, NULL))
			return ARCHIVE_EOF;
	        *extra_data_size -= name_size + 1;
		if(!read_ahead(a, name_size, &p))
			return ARCHIVE_EOF;
		if (name_size >= OWNER_MAXNAMELEN)
			name_len = OWNER_MAXNAMELEN - 1;
		else
			name_len = name_size;
		memcpy(namebuf, p, name_len);
		namebuf[name_len] = 0;
		if(ARCHIVE_OK != consume(a, (int64_t)name_size))
			return ARCHIVE_EOF;

		archive_entry_set_gname(entry, namebuf);
	}
	if ((flags & OWNER_USER_UID) != 0) {
		if(!read_var(a, &id, &value_size))
			return ARCHIVE_EOF;
		if(ARCHIVE_OK != consume(a, (int64_t)value_size))
			return ARCHIVE_EOF;
		*extra_data_size -= value_size;

		archive_entry_set_uid(entry, (la_int64_t)id);
	}
	if ((flags & OWNER_GROUP_GID) != 0) {
		if(!read_var(a, &id, &value_size))
			return ARCHIVE_EOF;
		if(ARCHIVE_OK != consume(a, (int64_t)value_size))
			return ARCHIVE_EOF;
		*extra_data_size -= value_size;

		archive_entry_set_gid(entry, (la_int64_t)id);
	}
	return ARCHIVE_OK;