static int
coolkey_add_object(coolkey_private_data_t *priv, unsigned long object_id, const u8 *object_data, size_t object_length, int add_v1_record)
{
	sc_cardctl_coolkey_object_t new_object;
	int r;

	memset(&new_object, 0, sizeof(new_object));
	new_object.path = coolkey_template_path;
	new_object.path.len = 4;
	ulong2bebytes(new_object.path.value, object_id);
	new_object.id = object_id;
	new_object.length = object_length;

	/* The object ID needs to be unique */
	if (coolkey_find_object_by_id(&priv->objects_list, object_id) != NULL) {
		return SC_ERROR_INTERNAL;
	}

	if (object_data) {
		new_object.data = malloc(object_length + add_v1_record);
		if (new_object.data == NULL) {
			return SC_ERROR_OUT_OF_MEMORY;
		}
		if (add_v1_record) {
			new_object.data[0] = COOLKEY_V1_OBJECT;
			new_object.length++;
		}
		memcpy(&new_object.data[add_v1_record], object_data, object_length);
	}

	r = coolkey_add_object_to_list(&priv->objects_list, &new_object);
	if (r != SC_SUCCESS) {
		/* if we didn't successfully put the object on the list,
		 * the data space didn't get adopted. free it before we return */
		free(new_object.data);
		new_object.data = NULL;
	}
	return r;
}