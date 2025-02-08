/* Required so finaliser can work when result freed. */
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