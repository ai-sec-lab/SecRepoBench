label(gx_monitor_alloc(memory->stable_memory),
                                    "gsicc_cache_new");
    if (result->lock == NULL) {
        gs_free_object(memory->stable_memory, result, "gsicc_cache_new");
        return(NULL);
    }
    result->full_wait = gx_semaphore_label(gx_semaphore_alloc(memory->stable_memory),
                                    "gsicc_cache_new");
    if (result->full_wait == NULL) {
        gx_monitor_free(result->lock);
        gs_free_object(memory->stable_memory, result, "gsicc_cache_new");
        return(NULL);
    }