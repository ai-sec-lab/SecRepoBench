static int general_endcidrange_func(gs_memory_t *mem, pdf_ps_ctx_t *s, pdf_cmap *pdficmap, pdfi_cmap_range_t *cmap_range)
{
    int ncodemaps, to_pop = pdf_ps_stack_count_to_mark(s, PDF_PS_OBJ_MARK);
    int i, bytesNeeded;
    pdfi_cmap_range_map_t *pdfir;
    pdf_ps_stack_object_t *stobj;

    /* increment to_pop to cover the mark object */
    ncodemaps = to_pop++;
    /* mapping should have 3 objects on the stack:
     * startcode, endcode and basecid
     */
    while (ncodemaps % 3) ncodemaps--;

    stobj = &s->cur[-ncodemaps] + 1;

    for (i = 0; i < ncodemaps; i += 3) {
        int preflen, valuelen;

        if (pdf_ps_obj_has_type(&(stobj[i + 2]), PDF_PS_OBJ_INTEGER)
        &&  pdf_ps_obj_has_type(&(stobj[i + 1]), PDF_PS_OBJ_STRING)
        &&  pdf_ps_obj_has_type(&(stobj[i]), PDF_PS_OBJ_STRING)){
            uint cidbase = stobj[i + 2].val.i;

            /* First, find the length of the prefix */
            for (preflen = 0; preflen < stobj[i].size; preflen++) {
                if(stobj[i].val.string[preflen] != stobj[i + 1].val.string[preflen]) {
                    break;
                }
            }

            if (preflen == stobj[i].size) {
                preflen = 1;
            }

            // <MASK>
        }
    }
    return pdf_ps_stack_pop(s, to_pop);
}