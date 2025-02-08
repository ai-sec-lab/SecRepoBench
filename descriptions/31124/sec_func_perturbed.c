static void cil_reset_perm(struct cil_perm *permission)
{
	cil_list_destroy(&permission->classperms, CIL_FALSE);
}