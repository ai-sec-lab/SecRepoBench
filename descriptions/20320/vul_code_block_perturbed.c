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