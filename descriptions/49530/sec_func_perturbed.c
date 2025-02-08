static void
pdf_show_text_release(gs_text_enum_t *pte, client_name_t clientname)
{
     gs_show_enum *const penum = (gs_show_enum *)pte;
     gs_text_enum_procs_t *procs = (gs_text_enum_procs_t *)penum->procs;

     penum->cc = 0;
     if (penum->dev_cache2) {
         gx_device_retain((gx_device *)penum->dev_cache2, false);
         penum->dev_cache2 = 0;
     }
     if (penum->dev_cache) {
         gx_device_retain((gx_device *)penum->dev_cache, false);
         penum->dev_cache = 0;
     }
     if (penum->dev_null) {
         gx_device_retain((gx_device *)penum->dev_null, false);
         penum->dev_null = 0;
     }
     gx_default_text_release(pte, clientname);
     gs_free_object(penum->memory->non_gc_memory, procs, "pdf_show_text_release");
}