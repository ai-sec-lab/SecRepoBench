static void user_stream_create_object(struct php_user_stream_wrapper *uwrap, php_stream_context *context, zval *objectinstance)
{
	if (uwrap->ce->ce_flags & (ZEND_ACC_INTERFACE|ZEND_ACC_TRAIT|ZEND_ACC_IMPLICIT_ABSTRACT_CLASS|ZEND_ACC_EXPLICIT_ABSTRACT_CLASS)) {
		ZVAL_UNDEF(objectinstance);
		return;
	}

	/* create an instance of our class */
	if (object_init_ex(objectinstance, uwrap->ce) == FAILURE) {
		ZVAL_UNDEF(objectinstance);
		return;
	}

	if (context) {
		// <MASK>
	} else {
		add_property_null(objectinstance, "context");
	}

	if (uwrap->ce->constructor) {
		zend_call_known_instance_method_with_0_params(
			uwrap->ce->constructor, Z_OBJ_P(objectinstance), NULL);
	}
}