if (!datum->fqn) {
		cil_tree_log(current, CIL_ERR, "Anonymous classpermission used in a classpermissionset");
		rc = SEPOL_ERR;
		goto exit;
	}

	rc = cil_resolve_classperms_list(current, cps->classperms, db);
	if (rc != SEPOL_OK) {
		goto exit;
	}

	cp = (struct cil_classpermission *)datum;
	cps->set = cp;

	if (cp->classperms == NULL) {
		cil_list_init(&cp->classperms, CIL_CLASSPERMS);
	}

	cil_list_for_each(curr, cps->classperms) {
		cil_list_append(cp->classperms, curr->flavor, curr->data);
	}

	return SEPOL_OK;

exit:
	return rc;