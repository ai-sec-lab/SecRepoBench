static void cil_reset_perm(struct cil_perm *perm)
{
	cil_list_destroy(&perm->classperms, CIL_FALSE);
}