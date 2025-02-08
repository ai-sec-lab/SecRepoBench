static int
coolkey_add_object(coolkey_private_data_t *priv, unsigned long objid, const u8 *object_data, size_t object_length, int add_v1_record)
{
	sc_cardctl_coolkey_object_t new_object;
	int r;

	memset(&new_object, 0, sizeof(new_object));
	new_object.path = coolkey_template_path;
	new_object.path.len = 4;
	ulong2bebytes(new_object.path.value, objid);
	new_object.id = objid;
	new_object.length = object_length;

	/* The object ID needs to be unique */
	// <MASK>
}