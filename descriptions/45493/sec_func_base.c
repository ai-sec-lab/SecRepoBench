int cond_init_bool_indexes(policydb_t * p)
{
	if (p->bool_val_to_struct)
		free(p->bool_val_to_struct);
	p->bool_val_to_struct = (cond_bool_datum_t **)
	    calloc(p->p_bools.nprim, sizeof(cond_bool_datum_t *));
	if (!p->bool_val_to_struct)
		return -1;
	return 0;
}