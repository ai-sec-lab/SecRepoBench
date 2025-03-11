zval *pcount, *pmembers;
	spl_SplObjectStorageElement *element;
	zend_long count;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &serialized_data_buffer, &buf_len) == FAILURE) {
		RETURN_THROWS();
	}

	if (buf_len == 0) {
		return;
	}

	/* storage */
	s = p = (const unsigned char*)serialized_data_buffer;
	PHP_VAR_UNSERIALIZE_INIT(var_hash);

	if (*p!= 'x' || *++p != ':') {
		goto outexcept;
	}
	++p;

	pcount = var_tmp_var(&var_hash);
	if (!php_var_unserialize(pcount, &p, s + buf_len, &var_hash) || Z_TYPE_P(pcount) != IS_LONG) {
		goto outexcept;
	}

	--p; /* for ';' */
	count = Z_LVAL_P(pcount);
	if (count < 0) {
		goto outexcept;
	}

	while (count-- > 0) {
		spl_SplObjectStorageElement *pelement;
		zend_hash_key key;
		zval *entry = var_tmp_var(&var_hash);
		zval inf;
		ZVAL_UNDEF(&inf);

		if (*p != ';') {
			goto outexcept;
		}
		++p;
		if(*p != 'O' && *p != 'C' && *p != 'r') {
			goto outexcept;
		}
		/* store reference to allow cross-references between different elements */
		if (!php_var_unserialize(entry, &p, s + buf_len, &var_hash)) {
			goto outexcept;
		}
		if (*p == ',') { /* new version has inf */
			++p;
			if (!php_var_unserialize(&inf, &p, s + buf_len, &var_hash)) {
				zval_ptr_dtor(&inf);
				goto outexcept;
			}
		}
		if (Z_TYPE_P(entry) != IS_OBJECT) {
			zval_ptr_dtor(&inf);
			goto outexcept;
		}

		if (spl_object_storage_get_hash(&key, intern, Z_OBJ_P(entry)) == FAILURE) {
			zval_ptr_dtor(&inf);
			goto outexcept;
		}
		pelement = spl_object_storage_get(intern, &key);
		spl_object_storage_free_hash(intern, &key);
		if (pelement) {
			zval obj;
			if (!Z_ISUNDEF(pelement->inf)) {
				var_push_dtor(&var_hash, &pelement->inf);
			}
			ZVAL_OBJ(&obj, pelement->obj);
			var_push_dtor(&var_hash, &obj);
		}
		element = spl_object_storage_attach(intern, Z_OBJ_P(entry), Z_ISUNDEF(inf)?NULL:&inf);
		var_replace(&var_hash, &inf, &element->inf);
		zval_ptr_dtor(&inf);
	}