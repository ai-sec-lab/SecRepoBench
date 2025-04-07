PHP_METHOD(SplObjectStorage, unserialize)
{
	spl_SplObjectStorage *intern = Z_SPLOBJSTORAGE_P(ZEND_THIS);

	char *serialized_data_buffer;
	size_t buf_len;
	const unsigned char *p, *s;
	php_unserialize_data_t var_hash;
	// Parse the input parameters to retrieve the serialized buffer and its length.
	// Initialize variables needed for unserialization, including a hash for variable tracking.
	// Check for expected buffer format and unserialize the count of elements in the buffer.
	// Iterate through the serialized elements, checking for the correct format of each entry.
	// For each element, unserialize it and optionally its associated information if present.
	// Validate the unserialized data, ensuring it is an object, and retrieve or create a storage element.
	// Manage references and memory for the unserialized entries, facilitating cross-references.
	// Attach the unserialized elements and their information to the internal storage structure.
	// <MASK>

	if (*p != ';') {
		goto outexcept;
	}
	++p;

	/* members */
	if (*p!= 'm' || *++p != ':') {
		goto outexcept;
	}
	++p;

	pmembers = var_tmp_var(&var_hash);
	if (!php_var_unserialize(pmembers, &p, s + buf_len, &var_hash) || Z_TYPE_P(pmembers) != IS_ARRAY) {
		goto outexcept;
	}

	/* copy members */
	object_properties_load(&intern->std, Z_ARRVAL_P(pmembers));

	PHP_VAR_UNSERIALIZE_DESTROY(var_hash);
	return;

outexcept:
	PHP_VAR_UNSERIALIZE_DESTROY(var_hash);
	zend_throw_exception_ex(spl_ce_UnexpectedValueException, 0, "Error at offset %zd of %zd bytes", ((char*)p - serialized_data_buffer), buf_len);
	RETURN_THROWS();

} /* }}} */