ZEND_API zend_attribute *zend_add_attribute(HashTable **attributes, zend_bool persistent, uint32_t offset, zend_string *attribute, uint32_t argc)
{
	if (*attributes == NULL) {
		*attributes = pemalloc(sizeof(HashTable), persistent);
		zend_hash_init(*attributes, 8, NULL, persistent ? attr_pfree : attr_free, persistent);
	}

	zend_attribute *attr = pemalloc(ZEND_ATTRIBUTE_SIZE(argc), persistent);

	// Set the attribute name by either copying or duplicating the given name based on persistence.
	// Convert the attribute name to lowercase and store it.
	// Assign the offset and argument count to the attribute structure.
	// Insert the new attribute into the attributes hash table and return a pointer to it.
	// <MASK>
}