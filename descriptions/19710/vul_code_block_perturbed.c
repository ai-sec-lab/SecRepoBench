zend_class_entry *ce, *scope;
	zend_class_constant *c;
	zval *value, *zv;
	USE_OPLINE

	SAVE_OPLINE();

	do {
		if (OP1_TYPE == IS_CONST) {
			if (EXPECTED(CACHED_PTR(opline->extended_value + sizeof(void*)))) {
				value = CACHED_PTR(opline->extended_value + sizeof(void*));
				break;
			} else if (EXPECTED(CACHED_PTR(opline->extended_value))) {
				ce = CACHED_PTR(opline->extended_value);
			} else {
				ce = zend_fetch_class_by_name(Z_STR_P(RT_CONSTANT(opline, opline->op1)), Z_STR_P(RT_CONSTANT(opline, opline->op1) + 1), ZEND_FETCH_CLASS_DEFAULT | ZEND_FETCH_CLASS_EXCEPTION);
				if (UNEXPECTED(ce == NULL)) {
					ZEND_ASSERT(EG(exception));
					ZVAL_UNDEF(EX_VAR(opline->result.var));
					HANDLE_EXCEPTION();
				}
			}
		} else {
			if (OP1_TYPE == IS_UNUSED) {
				ce = zend_fetch_class(NULL, opline->op1.num);
				if (UNEXPECTED(ce == NULL)) {
					ZEND_ASSERT(EG(exception));
					ZVAL_UNDEF(EX_VAR(opline->result.var));
					HANDLE_EXCEPTION();
				}
			} else {
				ce = Z_CE_P(EX_VAR(opline->op1.var));
			}
			if (EXPECTED(CACHED_PTR(opline->extended_value) == ce)) {
				value = CACHED_PTR(opline->extended_value + sizeof(void*));
				break;
			}
		}

		zv = zend_hash_find_ex(&ce->constants_table, Z_STR_P(RT_CONSTANT(opline, opline->op2)), 1);
		if (EXPECTED(zv != NULL)) {
			c = Z_PTR_P(zv);
			scope = EX(func)->op_array.scope;
			if (!zend_verify_const_access(c, scope)) {
				zend_throw_error(NULL, "Cannot access %s const %s::%s", zend_visibility_string(Z_ACCESS_FLAGS(c->value)), ZSTR_VAL(ce->name), Z_STRVAL_P(RT_CONSTANT(opline, opline->op2)));
				ZVAL_UNDEF(EX_VAR(opline->result.var));
				HANDLE_EXCEPTION();
			}
			value = &c->value;
			if (Z_TYPE_P(value) == IS_CONSTANT_AST) {
				zval_update_constant_ex(value, c->ce);
				if (UNEXPECTED(EG(exception) != NULL)) {
					ZVAL_UNDEF(EX_VAR(opline->result.var));
					HANDLE_EXCEPTION();
				}
			}
			CACHE_POLYMORPHIC_PTR(opline->extended_value, ce, value);
		} else {
			zend_throw_error(NULL, "Undefined class constant '%s'", Z_STRVAL_P(RT_CONSTANT(opline, opline->op2)));
			ZVAL_UNDEF(EX_VAR(opline->result.var));
			HANDLE_EXCEPTION();
		}
	} while (0);

	ZVAL_COPY_OR_DUP(EX_VAR(opline->result.var), value);

	ZEND_VM_NEXT_OPCODE();