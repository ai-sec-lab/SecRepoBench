if (FAILURE == zend_get_attribute_value(&attributesflags, attr, 0, scope)) {
			return;
		}

		if (Z_TYPE(attributesflags) != IS_LONG) {
			zend_error_noreturn(E_ERROR,
				"Attribute::__construct(): Argument #1 ($flags) must must be of type int, %s given",
				zend_zval_type_name(&attributesflags)
			);
		}

		if (Z_LVAL(attributesflags) & ~ZEND_ATTRIBUTE_FLAGS) {
			zend_error_noreturn(E_ERROR, "Invalid attribute flags specified");
		}

		zval_ptr_dtor(&attributesflags);