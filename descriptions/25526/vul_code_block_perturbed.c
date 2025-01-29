	USE_OPLINE
	zval *val;

	val = GET_OP1_ZVAL_PTR_UNDEF(BP_VAR_R);
	if (OP1_TYPE != IS_CONST) {
		ZVAL_DEREF(val);
	}

	if (Z_TYPE_INFO_P(val) > IS_NULL) {
		ZEND_VM_NEXT_OPCODE();
	} else {
		zval *result = EX_VAR(opline->result.var);

		if (EXPECTED(opline->extended_value == ZEND_SHORT_CIRCUITING_CHAIN_EXPR)) {
			if (UNEXPECTED(Z_TYPE_INFO_P(val) == IS_UNDEF)) {
				SAVE_OPLINE();
				ZVAL_UNDEFINED_OP1();
				if (UNEXPECTED(EG(exception) != NULL)) {
					HANDLE_EXCEPTION();
				}
			}

			ZVAL_NULL(result);
		} else if (opline->extended_value == ZEND_SHORT_CIRCUITING_CHAIN_ISSET) {
			ZVAL_FALSE(result);
		} else {
			ZEND_ASSERT(opline->extended_value == ZEND_SHORT_CIRCUITING_CHAIN_EMPTY);
			ZVAL_TRUE(result);
		}

		ZEND_VM_JMP_EX(OP_JMP_ADDR(opline, opline->op2), 0);
	}