void validate_attribute(zend_attribute *attr, uint32_t target, zend_class_entry *scope)
{
	// TODO: More proper signature validation: Too many args, incorrect arg names.
	if (attr->argc > 0) {
		zval attributesflags;

		// <MASK>
	}
}