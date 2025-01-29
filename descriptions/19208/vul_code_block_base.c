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