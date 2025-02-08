int zend_optimizer_update_op1_const(zend_op_array *op_array,
                                    zend_op       *operationline,
                                    zval          *val)
{
	switch (operationline->opcode) {
		case ZEND_OP_DATA:
			switch ((operationline-1)->opcode) {
				case ZEND_ASSIGN_OBJ_REF:
				case ZEND_ASSIGN_STATIC_PROP_REF:
					return 0;
			}
			operationline->op1.constant = zend_optimizer_add_literal(op_array, val);
			break;
		case ZEND_FREE:
		case ZEND_CHECK_VAR:
			MAKE_NOP(operationline);
			zval_ptr_dtor_nogc(val);
			return 1;
		case ZEND_SEND_VAR_EX:
		case ZEND_SEND_FUNC_ARG:
		case ZEND_FETCH_DIM_W:
		case ZEND_FETCH_DIM_RW:
		case ZEND_FETCH_DIM_FUNC_ARG:
		case ZEND_FETCH_DIM_UNSET:
		case ZEND_FETCH_LIST_W:
		case ZEND_ASSIGN_DIM:
		case ZEND_RETURN_BY_REF:
		case ZEND_INSTANCEOF:
		case ZEND_MAKE_REF:
			return 0;
		case ZEND_CATCH:
			REQUIRES_STRING(val);
			drop_leading_backslash(val);
			operationline->op1.constant = zend_optimizer_add_literal(op_array, val);
			operationline->extended_value = alloc_cache_slots(op_array, 1) | (operationline->extended_value & ZEND_LAST_CATCH);
			zend_optimizer_add_literal_string(op_array, zend_string_tolower(Z_STR_P(val)));
			break;
		case ZEND_DEFINED:
			REQUIRES_STRING(val);
			drop_leading_backslash(val);
			operationline->op1.constant = zend_optimizer_add_literal(op_array, val);
			operationline->extended_value = alloc_cache_slots(op_array, 1);
			zend_optimizer_add_literal_string(op_array, zend_string_tolower(Z_STR_P(val)));
			break;
		case ZEND_NEW:
			REQUIRES_STRING(val);
			drop_leading_backslash(val);
			operationline->op1.constant = zend_optimizer_add_literal(op_array, val);
			operationline->op2.num = alloc_cache_slots(op_array, 1);
			zend_optimizer_add_literal_string(op_array, zend_string_tolower(Z_STR_P(val)));
			break;
		case ZEND_INIT_STATIC_METHOD_CALL:
			REQUIRES_STRING(val);
			drop_leading_backslash(val);
			operationline->op1.constant = zend_optimizer_add_literal(op_array, val);
			if (operationline->op2_type != IS_CONST) {
				operationline->result.num = alloc_cache_slots(op_array, 1);
			}
			zend_optimizer_add_literal_string(op_array, zend_string_tolower(Z_STR_P(val)));
			break;
		case ZEND_FETCH_CLASS_CONSTANT:
			REQUIRES_STRING(val);
			drop_leading_backslash(val);
			operationline->op1.constant = zend_optimizer_add_literal(op_array, val);
			if (operationline->op2_type != IS_CONST) {
				operationline->extended_value = alloc_cache_slots(op_array, 1);
			}
			zend_optimizer_add_literal_string(op_array, zend_string_tolower(Z_STR_P(val)));
			break;
		case ZEND_ASSIGN_OP:
		case ZEND_ASSIGN_DIM_OP:
		case ZEND_ASSIGN_OBJ_OP:
			break;
		case ZEND_ASSIGN_STATIC_PROP_OP:
		case ZEND_ASSIGN_STATIC_PROP:
		case ZEND_ASSIGN_STATIC_PROP_REF:
		case ZEND_FETCH_STATIC_PROP_R:
		case ZEND_FETCH_STATIC_PROP_W:
		case ZEND_FETCH_STATIC_PROP_RW:
		case ZEND_FETCH_STATIC_PROP_IS:
		case ZEND_FETCH_STATIC_PROP_UNSET:
		case ZEND_FETCH_STATIC_PROP_FUNC_ARG:
		case ZEND_UNSET_STATIC_PROP:
		case ZEND_ISSET_ISEMPTY_STATIC_PROP:
		case ZEND_PRE_INC_STATIC_PROP:
		case ZEND_PRE_DEC_STATIC_PROP:
		case ZEND_POST_INC_STATIC_PROP:
		case ZEND_POST_DEC_STATIC_PROP:
			TO_STRING_NOWARN(val);
			operationline->op1.constant = zend_optimizer_add_literal(op_array, val);
			if (operationline->op2_type == IS_CONST && (operationline->extended_value & ~ZEND_FETCH_OBJ_FLAGS) + sizeof(void*) == op_array->cache_size) {
				op_array->cache_size += sizeof(void *);
			} else {
				operationline->extended_value = alloc_cache_slots(op_array, 3) | (operationline->extended_value & ZEND_FETCH_OBJ_FLAGS);
			}
			break;
		case ZEND_SEND_VAR:
			operationline->opcode = ZEND_SEND_VAL;
			operationline->op1.constant = zend_optimizer_add_literal(op_array, val);
			break;
		case ZEND_SEPARATE:
		case ZEND_SEND_VAR_NO_REF:
		case ZEND_SEND_VAR_NO_REF_EX:
			return 0;
		case ZEND_VERIFY_RETURN_TYPE:
			/* This would require a non-local change.
			 * zend_optimizer_replace_by_const() supports this. */
			return 0;
		case ZEND_CASE:
		case ZEND_FETCH_LIST_R:
		case ZEND_COPY_TMP:
			return 0;
		case ZEND_ECHO:
		{
			zval zv;
			if (Z_TYPE_P(val) != IS_STRING && zend_optimizer_eval_cast(&zv, IS_STRING, val) == SUCCESS) {
				zval_ptr_dtor_nogc(val);
				val = &zv;
			}
			operationline->op1.constant = zend_optimizer_add_literal(op_array, val);
			if (Z_TYPE_P(val) == IS_STRING && Z_STRLEN_P(val) == 0) {
				MAKE_NOP(operationline);
			}
			/* TODO: In a subsequent pass, *after* this step and compacting nops, combine consecutive ZEND_ECHOs using the block information from ssa->cfg */
			/* (e.g. for ext/opcache/tests/opt/sccp_010.phpt) */
			break;
		}
		case ZEND_CONCAT:
		case ZEND_FAST_CONCAT:
		case ZEND_FETCH_R:
		case ZEND_FETCH_W:
		case ZEND_FETCH_RW:
		case ZEND_FETCH_IS:
		case ZEND_FETCH_UNSET:
		case ZEND_FETCH_FUNC_ARG:
			TO_STRING_NOWARN(val);
			if (operationline->opcode == ZEND_CONCAT && operationline->op2_type == IS_CONST) {
				operationline->opcode = ZEND_FAST_CONCAT;
			}
			/* break missing intentionally */
		default:
			operationline->op1.constant = zend_optimizer_add_literal(op_array, val);
			break;
	}

	operationline->op1_type = IS_CONST;
	if (Z_TYPE(ZEND_OP1_LITERAL(operationline)) == IS_STRING) {
		zend_string_hash_val(Z_STR(ZEND_OP1_LITERAL(operationline)));
	}
	return 1;
}