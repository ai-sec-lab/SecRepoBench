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
	if (semanticlevel->sens > p->p_levels.nprim || !p->p_sens_val_to_name[semanticlevel->sens - 1])
		return -1;

	l->sens = semanticlevel->sens;
	levdatum = (level_datum_t *) hashtab_search(p->p_levels.table,
						    p->p_sens_val_to_name[l->sens - 1]);
	if (!levdatum) {
		ERR(h, "%s: Impossible situation found, nothing in p_levels.table.\n",
		    __func__);
		errno = ENOENT;
		return -1;
	}
	for (cat = semanticlevel->cat; cat; cat = cat->next) {
		if (cat->low > cat->high) {
			ERR(h, "Category range is not valid %s.%s",
			    p->p_cat_val_to_name[cat->low - 1],
			    p->p_cat_val_to_name[cat->high - 1]);
			return -1;
		}
		for (i = cat->low - 1; i < cat->high; i++) {
			if (!ebitmap_get_bit(&levdatum->level->cat, i)) {
				ERR(h, "Category %s can not be associated with "
				    "level %s",
				    p->p_cat_val_to_name[i],
				    p->p_sens_val_to_name[l->sens - 1]);
				return -1;
			}
			if (ebitmap_set_bit(&l->cat, i, 1)) {
				ERR(h, "Out of memory!");
				return -1;
			}
		}
	}

	return 0;
}