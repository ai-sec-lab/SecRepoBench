ZEND_FASTCALL concat_function(zval *result, zval *op1, zval *op2) /* {{{ */
{
	zval *orig_op1 = op1;
	zend_string *op1_string, *op2_string;
	bool shouldfreeopstring = false;
	bool free_op2_string = false;

	do {
		if (EXPECTED(Z_TYPE_P(op1) == IS_STRING)) {
			op1_string = Z_STR_P(op1);
		} else {
	 		if (Z_ISREF_P(op1)) {
	 			op1 = Z_REFVAL_P(op1);
				if (Z_TYPE_P(op1) == IS_STRING) {
					op1_string = Z_STR_P(op1);
					break;
				}
	 		}
			ZEND_TRY_BINARY_OBJECT_OPERATION(ZEND_CONCAT);
			op1_string = zval_get_string_func(op1);
			if (UNEXPECTED(EG(exception))) {
				zend_string_release(op1_string);
				if (orig_op1 != result) {
					ZVAL_UNDEF(result);
				}
				return FAILURE;
			}
			shouldfreeopstring = true;
			if (result == op1) {
				if (UNEXPECTED(op1 == op2)) {
					op2_string = op1_string;
					goto has_op2_string;
				}
			}
		}
	} while (0);
	do {
		if (EXPECTED(Z_TYPE_P(op2) == IS_STRING)) {
			op2_string = Z_STR_P(op2);
		} else {
			if (Z_ISREF_P(op2)) {
				op2 = Z_REFVAL_P(op2);
				if (Z_TYPE_P(op2) == IS_STRING) {
					op2_string = Z_STR_P(op2);
					break;
				}
			}
			/* hold an additional reference because a userland function could free this */
			if (!shouldfreeopstring) {
				op1_string = zend_string_copy(op1_string);
				shouldfreeopstring = true;
			}
			ZEND_TRY_BINARY_OP2_OBJECT_OPERATION(ZEND_CONCAT);
			op2_string = zval_get_string_func(op2);
			if (UNEXPECTED(EG(exception))) {
				zend_string_release(op1_string);
				zend_string_release(op2_string);
				if (orig_op1 != result) {
					ZVAL_UNDEF(result);
				}
				return FAILURE;
			}
			free_op2_string = true;
		}
	} while (0);

has_op2_string:;
	if (UNEXPECTED(ZSTR_LEN(op1_string) == 0)) {
		if (EXPECTED(result != op2 || Z_TYPE_P(result) != IS_STRING)) {
			if (result == orig_op1) {
				i_zval_ptr_dtor(result);
			}
			if (free_op2_string) {
				/* transfer ownership of op2_string */
				ZVAL_STR(result, op2_string);
				free_op2_string = false;
			} else {
				ZVAL_STR_COPY(result, op2_string);
			}
		}
	} else if (UNEXPECTED(ZSTR_LEN(op2_string) == 0)) {
		if (EXPECTED(result != op1 || Z_TYPE_P(result) != IS_STRING)) {
			if (result == orig_op1) {
				i_zval_ptr_dtor(result);
			}
			if (shouldfreeopstring) {
				/* transfer ownership of op1_string */
				ZVAL_STR(result, op1_string);
				shouldfreeopstring = false;
			} else {
				ZVAL_STR_COPY(result, op1_string);
			}
		}
	} else {
		size_t op1_len = ZSTR_LEN(op1_string);
		size_t op2_len = ZSTR_LEN(op2_string);
		size_t result_len = op1_len + op2_len;
		zend_string *result_str;
		uint32_t flags = ZSTR_GET_COPYABLE_CONCAT_PROPERTIES_BOTH(op1_string, op2_string);

		if (UNEXPECTED(op1_len > ZSTR_MAX_LEN - op2_len)) {
			if (shouldfreeopstring) zend_string_release(op1_string);
			if (free_op2_string) zend_string_release(op2_string);
			zend_throw_error(NULL, "String size overflow");
			if (orig_op1 != result) {
				ZVAL_UNDEF(result);
			}
			return FAILURE;
		}

		if (result == op1) {
			// Extend the memory allocation of op1_string to accommodate the concatenated result.
			// Assign the extended string to result_str, which will hold the final concatenated string.
			// If op1_string is being used as the result, ensure it is not prematurely freed.
			// Handle the case where op1_string and op2_string are the same, preventing double release.
			// Update op2_string to point to the new extended string if necessary.
			// <MASK>
		} else {
			result_str = zend_string_alloc(result_len, 0);
			memcpy(ZSTR_VAL(result_str), ZSTR_VAL(op1_string), op1_len);
			if (result == orig_op1) {
				i_zval_ptr_dtor(result);
			}
		}
		GC_ADD_FLAGS(result_str, flags);

		ZVAL_NEW_STR(result, result_str);
		memcpy(ZSTR_VAL(result_str) + op1_len, ZSTR_VAL(op2_string), op2_len);
		ZSTR_VAL(result_str)[result_len] = '\0';
	}

	if (shouldfreeopstring) zend_string_release(op1_string);
	if (free_op2_string) zend_string_release(op2_string);

	return SUCCESS;
}