ZEND_FASTCALL div_function(zval *result, zval *op1, zval *op2) /* {{{ */
{
	ZVAL_DEREF(op1);
	ZVAL_DEREF(op2);

	int retval = div_function_base(result, op1, op2);
	if (EXPECTED(retval == SUCCESS)) {
		return SUCCESS;
	}

	if (UNEXPECTED(retval == DIV_BY_ZERO)) {
		goto div_by_zero;
	}

	ZEND_TRY_BINARY_OBJECT_OPERATION(ZEND_DIV);

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

div_by_zero:
	ZEND_ASSERT(retval == DIV_BY_ZERO && "TYPES_NOT_HANDLED should not occur here");
	if (result != op1) {
		ZVAL_UNDEF(result);
	}
	zend_throw_error(zend_ce_division_by_zero_error, "Division by zero");
	return FAILURE;
}