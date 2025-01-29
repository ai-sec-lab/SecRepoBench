if (shouldfreeopstring) {
				/* op1_string will be used as the result, so we should not free it */
				i_zval_ptr_dtor(result);
				shouldfreeopstring = false;
			}
			/* special case, perform operations on result */
			result_str = zend_string_extend(op1_string, result_len, 0);
			/* account for the case where result_str == op1_string == op2_string and the realloc is done */