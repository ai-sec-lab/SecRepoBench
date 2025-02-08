static int pdfi_repair_add_object(pdf_context *context, int64_t obj, int64_t gen, gs_offset_t offset)
{
    /* Although we can handle object numbers larger than this, on some systems (32-bit Windows)
     * memset is limited to a (signed!) integer for the size of memory to clear. We could deal
     * with this by clearing the memory in blocks, but really, this is almost certainly a
     * corrupted file or something.
     */
    if (obj >= 0x7ffffff / sizeof(xref_entry) || obj < 1 || gen < 0 || offset < 0)
        return_error(gs_error_rangecheck);

    if (context->xref_table == NULL) {
        context->xref_table = (xref_table_t *)gs_alloc_bytes(context->memory, sizeof(xref_table_t), "repair xref table");
        if (context->xref_table == NULL) {
            return_error(gs_error_VMerror);
        }
        memset(context->xref_table, 0x00, sizeof(xref_table_t));
        context->xref_table->xref = (xref_entry *)gs_alloc_bytes(context->memory, (obj + 1) * sizeof(xref_entry), "repair xref table");
        if (context->xref_table->xref == NULL){
            gs_free_object(context->memory, context->xref_table, "failed to allocate xref table entries for repair");
            context->xref_table = NULL;
            return_error(gs_error_VMerror);
        }
        memset(context->xref_table->xref, 0x00, (obj + 1) * sizeof(xref_entry));
        context->xref_table->ctx = context;
        context->xref_table->type = PDF_XREF_TABLE;
        context->xref_table->xref_size = obj + 1;
#if REFCNT_DEBUG
        context->xref_table->UID = context->ref_UID++;
        dmprintf1(context->memory, "Allocated xref table with UID %"PRIi64"\n", context->xref_table->UID);
#endif
        pdfi_countup(context->xref_table);
    } else {
        if (context->xref_table->xref_size < (obj + 1)) {
            xref_entry *new_xrefs;

            new_xrefs = (xref_entry *)gs_alloc_bytes(context->memory, (obj + 1) * sizeof(xref_entry), "read_xref_stream allocate xref table entries");
            if (new_xrefs == NULL){
                pdfi_countdown(context->xref_table);
                context->xref_table = NULL;
                return_error(gs_error_VMerror);
            }
            memset(new_xrefs, 0x00, (obj + 1) * sizeof(xref_entry));
            memcpy(new_xrefs, context->xref_table->xref, context->xref_table->xref_size * sizeof(xref_entry));
            gs_free_object(context->memory, context->xref_table->xref, "reallocated xref entries");
            context->xref_table->xref = new_xrefs;
            context->xref_table->xref_size = obj + 1;
        }
    }
    context->xref_table->xref[obj].compressed = false;
    context->xref_table->xref[obj].free = false;
    context->xref_table->xref[obj].object_num = obj;
    context->xref_table->xref[obj].u.uncompressed.generation_num = gen;
    context->xref_table->xref[obj].u.uncompressed.offset = offset;
    return 0;
}