if (((tmp & MAY_BE_ARRAY) && (tmp & MAY_BE_ARRAY_KEY_ANY))
				 || opline->opcode == ZEND_FETCH_DIM_FUNC_ARG
				 || opline->opcode == ZEND_FETCH_DIM_R
				 || opline->opcode == ZEND_FETCH_DIM_IS
				 || opline->opcode == ZEND_FETCH_DIM_UNSET
				 || opline->opcode == ZEND_FETCH_LIST_R) {
					UPDATE_SSA_TYPE(tmp, ssa_op->op1_def);
				} else {
					/* invalid key type */
					return SUCCESS;
				}
				COPY_SSA_OBJ_TYPE(ssa_op->op1_use, ssa_op->op1_def);