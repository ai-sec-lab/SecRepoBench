static void cil_reset_classperms_set(struct cil_classperms_set *cp_set)
{
	if (cp_set == NULL) {
		return;
	}

	cp_set->set = NULL;
}