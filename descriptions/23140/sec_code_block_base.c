if (persistent == ((GC_FLAGS(name) & IS_STR_PERSISTENT) != 0)) {
		attr->name = zend_string_copy(name);
	} else {
		attr->name = zend_string_dup(name, persistent);
	}

	attr->lcname = zend_string_tolower_ex(attr->name, persistent);
	attr->offset = offset;
	attr->argc = argc;

	/* Initialize arguments to avoid partial initialization in case of fatal errors. */
	for (uint32_t i = 0; i < argc; i++) {
		ZVAL_UNDEF(&attr->argv[i]);
	}

	zend_hash_next_index_insert_ptr(*attributes, attr);

	return attr;