static void cil_reset_class(struct cil_class *cls)
{
	if (cls->common != NULL) {
		// <MASK>
		cls->common = NULL; /* Must make this NULL or there will be an error when re-resolving */
	}
	cls->ordered = CIL_FALSE;
}