static inline int pdf_ps_stack_push(pdf_ps_ctx_t *s)
{
    /* Extending the stack pretty inefficient, but it shouldn't happen often
       for valid files
     */
    if (s->cur + 1 >= s->toplim - 1) {
        int i, currsize = s->toplim - s->stack;
        int newsize = currsize + PDF_PS_STACK_GROW_SIZE;
        int newsizebytes = newsize * sizeof(pdf_ps_stack_object_t);
        pdf_ps_stack_object_t *nstack;

        if (newsize < PDF_PS_STACK_MAX) {
            nstack = (pdf_ps_stack_object_t *)gs_alloc_bytes(s->pdfi_ctx->memory, newsizebytes, "pdf_ps_stack_push(nstack)");
            if (nstack != NULL) {
                memcpy(nstack, s->stack, (currsize - 1) * sizeof(pdf_ps_stack_object_t));

                for (i = 0; i < PDF_PS_STACK_GUARDS; i++)
                    nstack[newsize - PDF_PS_STACK_GUARDS + i].type = PDF_PS_OBJ_STACK_TOP;

                for (i = currsize - 2; i < newsize - PDF_PS_STACK_GUARDS; i++) {
                    pdf_ps_make_null(&(nstack[i]));
                }

                gs_free_object(s->pdfi_ctx->memory, s->stack, "pdf_ps_stack_push(s->stack)");
                s->stack = nstack;
                s->cur = s->stack + currsize - 1;
                s->toplim = s->stack + newsize - PDF_PS_STACK_GROW_SIZE;
            }
            else {
                return_error(gs_error_VMerror);
            }
        }
    }
    s->cur++;
    if (pdf_ps_obj_has_type(s->cur, PDF_PS_OBJ_STACK_TOP))
        return_error(gs_error_stackoverflow);
    if (pdf_ps_obj_has_type(s->cur, PDF_PS_OBJ_STACK_BOTTOM))
        return_error(gs_error_stackunderflow);
    return 0;
}