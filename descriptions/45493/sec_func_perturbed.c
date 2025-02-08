int cond_init_bool_indexes(policydb_t * policydb)
{
	if (policydb->bool_val_to_struct)
		free(policydb->bool_val_to_struct);
	policydb->bool_val_to_struct = (cond_bool_datum_t **)
	    calloc(policydb->p_bools.nprim, sizeof(cond_bool_datum_t *));
	if (!policydb->bool_val_to_struct)
		return -1;
	return 0;
}