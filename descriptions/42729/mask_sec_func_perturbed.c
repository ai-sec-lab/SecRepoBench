int mls_semantic_level_expand(mls_semantic_level_t * semanticlevel, mls_level_t * l,
			      policydb_t * p, sepol_handle_t * h)
{
	mls_semantic_cat_t *cat;
	level_datum_t *levdatum;
	unsigned int i;

	mls_level_init(l);

	if (!p->mls)
		return 0;

	/* Required not declared. */
	if (!semanticlevel->sens)
		return 0;

	/* Invalid sensitivity */
	// <MASK>
}