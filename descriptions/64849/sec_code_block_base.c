if (!ebitmap_is_empty(&scope_index->scope[SYM_COMMONS]))
		goto bad;
	if (validate_ebitmap(&scope_index->p_classes_scope, &flavors[SYM_CLASSES]))
		goto bad;
	if (validate_ebitmap(&scope_index->p_roles_scope, &flavors[SYM_ROLES]))
		goto bad;
	if (validate_ebitmap(&scope_index->p_types_scope, &flavors[SYM_TYPES]))
		goto bad;
	if (validate_ebitmap(&scope_index->p_users_scope, &flavors[SYM_USERS]))
		goto bad;
	if (validate_ebitmap(&scope_index->p_bools_scope, &flavors[SYM_BOOLS]))
		goto bad;
	if (validate_ebitmap(&scope_index->p_sens_scope, &flavors[SYM_LEVELS]))
		goto bad;
	if (validate_ebitmap(&scope_index->p_cat_scope, &flavors[SYM_CATS]))
		goto bad;
	if (scope_index->class_perms_len > flavors[SYM_CLASSES].nprim)
		goto bad;

	return 0;

bad:
	ERR(handle, "Invalid scope");
	return -1;