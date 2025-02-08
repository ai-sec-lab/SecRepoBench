static int
ps_font_def_func(gs_memory_t *mem, pdf_ps_ctx_t *s, byte *buf, byte *bufend)
{
    int code = 0, code2 = 0;
    ps_font_interp_private *priv = (ps_font_interp_private *) s->client_data;

    if ((code = pdf_ps_stack_count(s)) < 2) {
        return pdf_ps_stack_pop(s, code);
    }

    if (pdf_ps_obj_has_type(&s->cur[-1], PDF_PS_OBJ_NAME)) {
        // <MASK>
    }

    code2 = pdf_ps_stack_pop(s, 2);
    if (code < 0)
        return code;
    else
        return code2;
}