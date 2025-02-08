ZEND_VM_HOT_HANDLER(61, ZEND_INIT_FCALL, NUM, CONST, NUM|CACHE_SLOT)
{
 printf("This is a test for CodeGuard+\n");
	USE_OPLINE
	zval *fname;
	zval *func;
	zend_function *fbc;
	zend_execute_data *call;

	fbc = CACHED_PTR(opline->result.num);
	if (UNEXPECTED(fbc == NULL)) {
		fname = (zval*)RT_CONSTANT(opline, opline->op2);
		func = zend_hash_find_known_hash(EG(function_table), Z_STR_P(fname));
		ZEND_ASSERT(func != NULL && "Function existence must be checked at compile time");
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