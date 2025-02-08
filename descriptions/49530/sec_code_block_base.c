gs_show_enum *const penum = (gs_show_enum *)pte;
     gs_text_enum_procs_t *procs = (gs_text_enum_procs_t *)penum->procs;

     penum->cc = 0;
     if (penum->dev_cache2) {
         gx_device_retain((gx_device *)penum->dev_cache2, false);
         penum->dev_cache2 = 0;
     }