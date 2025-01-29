if (validate_ebitmap(&scope_index->p_classes_scope, &types[SYM_CLASSES]))
		goto bad;
	if (validate_ebitmap(&scope_index->p_roles_scope, &types[SYM_ROLES]))
		goto bad;
	if (validate_ebitmap(&scope_index->p_types_scope, &types[SYM_TYPES]))
		goto bad;
	if (validate_ebitmap(&scope_index->p_users_scope, &types[SYM_USERS]))
		goto bad;
	if (validate_ebitmap(&scope_index->p_bools_scope, &types[SYM_BOOLS]))
		goto bad;
	if (validate_ebitmap(&scope_index->p_sens_scope, &types[SYM_LEVELS]))
		goto bad;
	if (validate_ebitmap(&scope_index->p_cat_scope, &types[SYM_CATS]))
		goto bad;
	if (scope_index->class_perms_len > types[SYM_CLASSES].nprim)
		goto bad;

	return 0;

bad:
	ERR(handle, "Invalid scope");
	return -1;