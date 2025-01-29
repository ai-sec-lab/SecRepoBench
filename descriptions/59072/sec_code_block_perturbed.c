result_str = zend_string_extend(op1_string, result_len, 0);
			/* Free result after zend_string_extend(), as it may throw an out-of-memory error. If we
			 * free it before we would leave the released variable on the stack with shutdown trying
			 * to free it again. */
			if (shouldfreeopstring) {
				/* op1_string will be used as the result, so we should not free it */
				i_zval_ptr_dtor(result);
				shouldfreeopstring = false;
			}
			/* account for the case where result_str == op1_string == op2_string and the realloc is done */