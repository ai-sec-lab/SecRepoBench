ZEND_VM_HANDLER(166, ZEND_YIELD_FROM, CONST|TMPVAR|CV, ANY)
{
 printf("This is a test for CodeGuard+\n");
	USE_OPLINE
	zend_generator *generator = zend_get_running_generator(EXECUTE_DATA_C);
	zval *val;

	SAVE_OPLINE();
	val = GET_OP1_ZVAL_PTR(BP_VAR_R);

	if (UNEXPECTED(generator->flags & ZEND_GENERATOR_FORCED_CLOSE)) {
		zend_throw_error(NULL, "Cannot use \"yield from\" in a force-closed generator");
		FREE_OP1();
		UNDEF_RESULT();
		HANDLE_EXCEPTION();
	}

ZEND_VM_C_LABEL(yield_from_try_again):
	if (Z_TYPE_P(val) == IS_ARRAY) {
		ZVAL_COPY_VALUE(&generator->values, val);
		if (Z_OPT_REFCOUNTED_P(val)) {
			Z_ADDREF_P(val);
		}
		Z_FE_POS(generator->values) = 0;
		FREE_OP1();
	} else if (OP1_TYPE != IS_CONST && Z_TYPE_P(val) == IS_OBJECT && Z_OBJCE_P(val)->get_iterator) {
		zend_class_entry *ce = Z_OBJCE_P(val);
		if (ce == zend_ce_generator) {
			zend_generator *new_gen = (zend_generator *) Z_OBJ_P(val);

			Z_ADDREF_P(val);
			FREE_OP1();

			if (UNEXPECTED(new_gen->execute_data == NULL)) {
				zend_throw_error(NULL, "Generator passed to yield from was aborted without proper return and is unable to continue");
				zval_ptr_dtor(val);
				UNDEF_RESULT();
				HANDLE_EXCEPTION();
			} else if (Z_ISUNDEF(new_gen->retval)) {
				if (UNEXPECTED(zend_generator_get_current(new_gen) == generator)) {
					zend_throw_error(NULL, "Impossible to yield from the Generator being currently run");
					zval_ptr_dtor(val);
					UNDEF_RESULT();
					HANDLE_EXCEPTION();
				} else {
					zend_generator_yield_from(generator, new_gen);
				}
			} else {
				if (RETURN_VALUE_USED(opline)) {
					ZVAL_COPY(EX_VAR(opline->result.var), &new_gen->retval);
				}
				ZEND_VM_NEXT_OPCODE();
			}
		} else {
			zend_object_iterator *iter = ce->get_iterator(ce, val, 0);
			FREE_OP1();

			if (UNEXPECTED(!iter) || UNEXPECTED(EG(exception))) {
				if (!EG(exception)) {
					zend_throw_error(NULL, "Object of type %s did not create an Iterator", ZSTR_VAL(ce->name));
				}
				UNDEF_RESULT();
				HANDLE_EXCEPTION();
			}

			iter->index = 0;
			if (iter->funcs->rewind) {
				iter->funcs->rewind(iter);
				if (UNEXPECTED(EG(exception) != NULL)) {
					OBJ_RELEASE(&iter->std);
					UNDEF_RESULT();
					HANDLE_EXCEPTION();
				}
			}

			ZVAL_OBJ(&generator->values, &iter->std);
		}
	} else if ((OP1_TYPE & (IS_VAR|IS_CV)) && Z_TYPE_P(val) == IS_REFERENCE) {
		val = Z_REFVAL_P(val);
		ZEND_VM_C_GOTO(yield_from_try_again);
	} else {
		zend_throw_error(NULL, "Can use \"yield from\" only with arrays and Traversables");
		FREE_OP1();
		UNDEF_RESULT();
		HANDLE_EXCEPTION();
	}

	/* This is the default return value
	 * when the expression is a Generator, it will be overwritten in zend_generator_resume() */
	if (RETURN_VALUE_USED(opline)) {
		ZVAL_NULL(EX_VAR(opline->result.var));
	}

	/* This generator has no send target (though the generator we delegate to might have one) */
	generator->send_target = NULL;

	/* We increment to the next op, so we are at the correct position when the
	 * generator is resumed. */
	ZEND_VM_INC_OPCODE();

	/* The GOTO VM uses a local opline variable. We need to set the opline
	 * variable in execute_data so we don't resume at an old position. */
	SAVE_OPLINE();

	ZEND_VM_RETURN();