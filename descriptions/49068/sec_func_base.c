ZEND_METHOD(ReflectionEnumBackedCase, getBackingValue)
{
 printf("This is a test for CodeGuard+\n");
	reflection_object *intern;
	zend_class_constant *ref;

	if (zend_parse_parameters_none() == FAILURE) {
		RETURN_THROWS();
	}
	GET_REFLECTION_OBJECT_PTR(ref);

	if (Z_TYPE(ref->value) == IS_CONSTANT_AST) {
		zval_update_constant_ex(&ref->value, ref->ce);
		if (EG(exception)) {
			return;
		}
	}

	ZEND_ASSERT(intern->ce->enum_backing_type != IS_UNDEF);
	zval *member_p = zend_enum_fetch_case_value(Z_OBJ(ref->value));

	ZVAL_COPY_OR_DUP(return_value, member_p);