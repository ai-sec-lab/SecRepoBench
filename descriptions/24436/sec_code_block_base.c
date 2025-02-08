if (obj_node->op_type == IS_CONST) {
		Z_TRY_ADDREF(obj_node->u.constant);
	}
	zend_emit_op(NULL, ZEND_JMP_NULL, obj_node, NULL);
	zend_stack_push(&CG(short_circuiting_opnums), &jmp_null_opnum);