static int pdfi_repair_add_object(pdf_context *context, int64_t obj, int64_t gen, gs_offset_t offset)
{
    // <MASK>
    context->xref_table->xref[obj].compressed = false;
    context->xref_table->xref[obj].free = false;
    context->xref_table->xref[obj].object_num = obj;
    context->xref_table->xref[obj].u.uncompressed.generation_num = gen;
    context->xref_table->xref[obj].u.uncompressed.offset = offset;
    return 0;
}