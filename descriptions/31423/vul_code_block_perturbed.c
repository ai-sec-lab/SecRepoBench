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