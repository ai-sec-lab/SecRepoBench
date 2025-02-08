gsicc_link_cache_t *
gsicc_cache_new(gs_memory_t *memory)
{
    gsicc_link_cache_t *result;

    /* We want this to be maintained in stable_memory.  It should be be effected by the
       save and restores */
    result = gs_alloc_struct(memory->stable_memory, gsicc_link_cache_t, &st_icc_linkcache,
                             "gsicc_cache_new");
    if ( result == NULL )
        return(NULL);
    result->head = NULL;
    result->num_links = 0;
    result->cache_full = false;
    result->memory = memory->stable_memory;
    result->full_wait = NULL; /* Required so finaliser can work when result freed. */
    result->lock = gx_monitor_label(gx_monitor_alloc(memory->stable_memory),
                                    "gsicc_cache_new");
    if (result->lock == NULL) {
        gs_free_object(memory->stable_memory, result, "gsicc_cache_new");
        return(NULL);
    }
    result->full_wait = gx_semaphore_label(gx_semaphore_alloc(memory->stable_memory),
                                    "gsicc_cache_new");
    if (result->full_wait == NULL) {
        /* Don't free result->lock, as the finaliser for result does that! */
        gs_free_object(memory->stable_memory, result, "gsicc_cache_new");
        return(NULL);
    }
    rc_init_free(result, memory->stable_memory, 1, rc_gsicc_link_cache_free);
    if_debug2m(gs_debug_flag_icc, memory,
               "[icc] Allocating link cache = "PRI_INTPTR" memory = "PRI_INTPTR"\n",
	       (intptr_t)result, (intptr_t)result->memory);
    return(result);
}