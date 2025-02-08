static void cil_reset_class(struct cil_class *cls)
{
	if (cls->common != NULL) {
		/* Must assume that the common has been destroyed */
		int num_common_perms = cls->num_perms - cls->perms.nprim;
		cil_symtab_map(&cls->perms, __class_reset_perm_values, &num_common_perms);
		/* during a re-resolve, we need to reset the common, so a classcommon
		 * statement isn't seen as a duplicate */
		cls->num_perms = cls->perms.nprim;
		cls->common = NULL; /* Must make this NULL or there will be an error when re-resolving */
	}
	cls->ordered = CIL_FALSE;
}