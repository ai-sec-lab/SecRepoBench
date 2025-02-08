static void cil_reset_classpermission(struct cil_classpermission *classperm)
{
	if (classperm == NULL) {
		return;
	}

	cil_list_destroy(&classperm->classperms, CIL_FALSE);
}