zval result_copy, op1_copy, op2_copy;
	if (UNEXPECTED(zendi_try_convert_scalar_to_number(op1, &op1_copy) == FAILURE)
			|| UNEXPECTED(zendi_try_convert_scalar_to_number(op2, &op2_copy) == FAILURE)) {
		zend_binop_error("/", op1, op2);
		if (result != op1) {
			ZVAL_UNDEF(result);
		}
		return FAILURE;
	}

	retval = div_function_base(&result_copy, &op1_copy, &op2_copy);
	if (retval == SUCCESS) {
		if (result == op1) {
			zval_ptr_dtor(result);
		}
		ZVAL_COPY_VALUE(result, &result_copy);
		return SUCCESS;
	}