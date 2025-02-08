static void cil_reset_class(struct cil_class *class)
{
	if (class->common != NULL) {
		// <MASK>
		class->common = NULL; /* Must make this NULL or there will be an error when re-resolving */
	}
	class->ordered = CIL_FALSE;
}