zval op1_copy, op2_copy;
	if (UNEXPECTED(zendi_try_convert_scalar_to_number(op1, &op1_copy) == FAILURE)
			|| UNEXPECTED(zendi_try_convert_scalar_to_number(op2, &op2_copy) == FAILURE)) {
		zend_binop_error("/", op1, op2);
		if (result != op1) {
			ZVAL_UNDEF(result);
		}
		return FAILURE;
	}

	if (result == op1) {
		zval_ptr_dtor(result);
	}

	retval = div_function_base(result, &op1_copy, &op2_copy);
	if (retval == SUCCESS) {
		return SUCCESS;
	}