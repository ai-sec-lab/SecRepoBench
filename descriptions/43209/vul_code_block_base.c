
	rc = cil_resolve_name(current, clscom->common_str, CIL_SYM_COMMONS, extra_args, &common_datum);
	if (rc != SEPOL_OK) {
		goto exit;
	}

	class = (struct cil_class *)class_datum;
	common = (struct cil_class *)common_datum;
	if (class->common != NULL) {
		cil_log(CIL_ERR, "class cannot be associeated with more than one common\n");
		rc = SEPOL_ERR;
		goto exit;
	}

	class->common = common;

	cil_symtab_map(&class->perms, __class_update_perm_values, &common->num_perms);

	class->num_perms += common->num_perms;
	if (class->num_perms > CIL_PERMS_PER_CLASS) {
		cil_tree_log(current, CIL_ERR, "Too many permissions in class '%s' when including common permissions", class->datum.name);
		rc = SEPOL_ERR;
		goto exit;
	}

	return SEPOL_OK;

exit:
	return rc;