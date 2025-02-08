static void cil_reset_classpermission(struct cil_classpermission *cp)
{
	if (cp == NULL) {
		return;
	}

	cil_list_destroy(&cp->classperms, CIL_FALSE);
}