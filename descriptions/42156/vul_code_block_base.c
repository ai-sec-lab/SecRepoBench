	USE_OPLINE
	zval *fname;
	zval *func;
	zend_function *fbc;
	zend_execute_data *call;

	fbc = CACHED_PTR(opline->result.num);
	if (UNEXPECTED(fbc == NULL)) {
		fname = (zval*)RT_CONSTANT(opline, opline->op2);
		func = zend_hash_find_known_hash(EG(function_table), Z_STR_P(fname));
		if (UNEXPECTED(func == NULL)) {
			ZEND_VM_DISPATCH_TO_HELPER(zend_undefined_function_helper);
		}
		fbc = Z_FUNC_P(func);
		if (EXPECTED(fbc->type == ZEND_USER_FUNCTION) && UNEXPECTED(!RUN_TIME_CACHE(&fbc->op_array))) {
			init_func_run_time_cache(&fbc->op_array);
		}
		CACHE_PTR(opline->result.num, fbc);
	}

	call = _zend_vm_stack_push_call_frame_ex(
		opline->op1.num, ZEND_CALL_NESTED_FUNCTION,
		fbc, opline->extended_value, NULL);
	call->prev_execute_data = EX(call);
	EX(call) = call;

	ZEND_VM_NEXT_OPCODE();