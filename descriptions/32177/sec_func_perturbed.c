static void cil_reset_classperms_set(struct cil_classperms_set *permissionsset)
{
	if (permissionsset == NULL) {
		return;
	}

	permissionsset->set = NULL;
}