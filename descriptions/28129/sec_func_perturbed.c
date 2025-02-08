void validate_attribute(zend_attribute *attr, uint32_t target, zend_class_entry *scope)
{
	// TODO: More proper signature validation: Too many args, incorrect arg names.
	if (attr->argc > 0) {
		zval attributesflags;

		/* As this is run in the middle of compilation, fetch the attribute value without
		 * specifying a scope. The class is not fully linked yet, and we may seen an
		 * inconsistent state. */
		if (FAILURE == zend_get_attribute_value(&attributesflags, attr, 0, NULL)) {
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
	}
}