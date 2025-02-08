zend_emit_op(NULL, ZEND_JMP_NULL, objoperand, NULL);
	zend_stack_push(&CG(short_circuiting_opnums), &jmp_null_opnum);