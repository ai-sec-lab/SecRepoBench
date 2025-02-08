gsicc_link_cache_t *
gsicc_cache_new(gs_memory_t *memallocator)
{
    gsicc_link_cache_t *result;

    /* We want this to be maintained in stable_memory.  It should be be effected by the
       save and restores */
    result = gs_alloc_struct(memallocator->stable_memory, gsicc_link_cache_t, &st_icc_linkcache,
                             "gsicc_cache_new");
    if ( result == NULL )
        return(NULL);
    result->head = NULL;
    result->num_links = 0;
    result->cache_full = false;
    result->memory = memallocator->stable_memory;
    result->full_wait = NULL; // <MASK>
    rc_init_free(result, memallocator->stable_memory, 1, rc_gsicc_link_cache_free);
    if_debug2m(gs_debug_flag_icc, memallocator,
               "[icc] Allocating link cache = "PRI_INTPTR" memory = "PRI_INTPTR"\n",
	       (intptr_t)result, (intptr_t)result->memory);
    return(result);
}