static void user_stream_create_object(struct php_user_stream_wrapper *uwrap, php_stream_context *context, zval *object)
{
	if (uwrap->ce->ce_flags & (ZEND_ACC_INTERFACE|ZEND_ACC_TRAIT|ZEND_ACC_IMPLICIT_ABSTRACT_CLASS|ZEND_ACC_EXPLICIT_ABSTRACT_CLASS)) {
		ZVAL_UNDEF(object);
		return;
	}

	/* create an instance of our class */
	if (object_init_ex(object, uwrap->ce) == FAILURE) {
		ZVAL_UNDEF(object);
		return;
	}

	if (context) {
		GC_ADDREF(context->res);
		add_property_resource(object, "context", context->res);
	} else {
		add_property_null(object, "context");
	}

	if (uwrap->ce->constructor) {
		zend_call_known_instance_method_with_0_params(
			uwrap->ce->constructor, Z_OBJ_P(object), NULL);
	}
}