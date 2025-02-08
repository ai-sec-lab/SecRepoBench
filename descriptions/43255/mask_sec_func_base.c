static int
pdf_ps_put_oper_func(gs_memory_t *mem, pdf_ps_ctx_t *s, byte *buf, byte *bufend)
{
    int code;

    if ((code = pdf_ps_stack_count(s)) < 4) {
        return 0;
    }

    if // <MASK>

    code = pdf_ps_stack_pop(s, 2);
    return code;
}