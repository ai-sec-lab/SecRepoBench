static void zend_emit_jmp_null(znode *objoperand)
{
	uint32_t jmp_null_opnum = get_next_op_number();
	if (objoperand->op_type == IS_CONST) {
		Z_TRY_ADDREF(objoperand->u.constant);
	}
	zend_emit_op(NULL, ZEND_JMP_NULL, objoperand, NULL);
	zend_stack_push(&CG(short_circuiting_opnums), &jmp_null_opnum);
}