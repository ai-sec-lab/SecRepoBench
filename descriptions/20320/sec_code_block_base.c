case ZEND_ECHO:
		{
			zval zv;
			if (Z_TYPE_P(val) != IS_STRING && zend_optimizer_eval_cast(&zv, IS_STRING, val) == SUCCESS) {
				zval_ptr_dtor_nogc(val);
				val = &zv;
			}
			opline->op1.constant = zend_optimizer_add_literal(op_array, val);
			if (Z_TYPE_P(val) == IS_STRING && Z_STRLEN_P(val) == 0) {
				MAKE_NOP(opline);
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
			if (opline->opcode == ZEND_CONCAT && opline->op2_type == IS_CONST) {
				opline->opcode = ZEND_FAST_CONCAT;
			}
			/* break missing intentionally */
		default:
			opline->op1.constant = zend_optimizer_add_literal(op_array, val);
			break;