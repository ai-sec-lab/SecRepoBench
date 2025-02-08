static int validate_cond_list(sepol_handle_t *handle, cond_list_t *cond, validate_t flavors[])
{
	for (; cond; cond = cond->next) {
		// <MASK>
	}

	return 0;

bad:
	ERR(handle, "Invalid cond list");
	return -1;
}