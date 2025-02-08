ZEND_FASTCALL zend_handle_undef_args(zend_execute_data *call) {
	zend_function *fbc = call->func;
	if (fbc->type == ZEND_USER_FUNCTION) {
		zend_op_array *op_array = &fbc->op_array;
		uint32_t num_args = ZEND_CALL_NUM_ARGS(call);
		for (uint32_t i = 0; i < num_args; i++) {
			zval *arg = ZEND_CALL_VAR_NUM(call, i);
			if (!Z_ISUNDEF_P(arg)) {
				continue;
			}

			zend_op *opline = &op_array->opcodes[i];
			if (EXPECTED(opline->opcode == ZEND_RECV_INIT)) {
				zval *default_value = RT_CONSTANT(opline, opline->op2);
				if (Z_OPT_TYPE_P(default_value) == IS_CONSTANT_AST) {
					if (UNEXPECTED(!RUN_TIME_CACHE(op_array))) {
						init_func_run_time_cache(op_array);
					}

					void *run_time_cache = RUN_TIME_CACHE(op_array);
					zval *cache_val =
						(zval *) ((char *) run_time_cache + Z_CACHE_SLOT_P(default_value));

					if (Z_TYPE_P(cache_val) != IS_UNDEF) {
						/* We keep in cache only not refcounted values */
						ZVAL_COPY_VALUE(arg, cache_val);
					} else {
						/* Update constant inside a temporary zval, to make sure the CONSTANT_AST
						 * value is not accessible through back traces. */
						zval tmp;
						ZVAL_COPY(&tmp, default_value);
						zend_execute_data *old = start_fake_frame(call, opline);
						zend_result ret = zval_update_constant_ex(&tmp, fbc->op_array.scope);
						end_fake_frame(call, old);
						if (UNEXPECTED(ret == FAILURE)) {
							zval_ptr_dtor_nogc(&tmp);
							return FAILURE;
						}
						ZVAL_COPY_VALUE(arg, &tmp);
						if (!Z_REFCOUNTED(tmp)) {
							ZVAL_COPY_VALUE(cache_val, &tmp);
						}
					}
				} else {
					ZVAL_COPY(arg, default_value);
				}
			} else {
				ZEND_ASSERT(opline->opcode == ZEND_RECV);
				zend_execute_data *old = start_fake_frame(call, opline);
				zend_argument_error(zend_ce_argument_count_error, i + 1, "not passed");
				end_fake_frame(call, old);
				return FAILURE;
			}
		}

		return SUCCESS;
	} else {
		if (fbc->common.fn_flags & ZEND_ACC_USER_ARG_INFO) {
			/* Magic function, let it deal with it. */
			return SUCCESS;
		}

		uint32_t num_args = ZEND_CALL_NUM_ARGS(call);
		for (uint32_t i = 0; i < num_args; i++) {
			zval *arg = ZEND_CALL_VAR_NUM(call, i);
			if (!Z_ISUNDEF_P(arg)) {
				continue;
			}

			zend_internal_arg_info *arg_info = &fbc->internal_function.arg_info[i];
			if (i < fbc->common.required_num_args) {
				zend_execute_data *old = start_fake_frame(call, NULL);
				zend_argument_error(zend_ce_argument_count_error, i + 1, "not passed");
				end_fake_frame(call, old);
				return FAILURE;
			}

			zval default_value;
			if (zend_get_default_from_internal_arg_info(&default_value, arg_info) == FAILURE) {
				zend_execute_data *old = start_fake_frame(call, NULL);
				zend_argument_error(zend_ce_argument_count_error, i + 1,
					"must be passed explicitly, because the default value is not known");
				end_fake_frame(call, old);
				return FAILURE;
			}

			if (Z_TYPE(default_value) == IS_CONSTANT_AST) {
				zend_execute_data *old = start_fake_frame(call, NULL);
				zend_result ret = zval_update_constant_ex(&default_value, fbc->common.scope);
				end_fake_frame(call, old);
				if (ret == FAILURE) {
					return FAILURE;
				}
			}

			ZVAL_COPY_VALUE(arg, &default_value);
			if (ZEND_ARG_SEND_MODE(arg_info) & ZEND_SEND_BY_REF) {
				ZVAL_NEW_REF(arg, arg);
			}
		}
	}

	return SUCCESS;
}