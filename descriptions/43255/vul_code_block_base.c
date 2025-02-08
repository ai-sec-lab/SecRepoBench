(pdf_ps_obj_has_type(&s->cur[-3], PDF_PS_OBJ_NAME) &&
        !memcmp(s->cur[-3].val.name, PDF_PS_OPER_NAME_AND_LEN("Encoding")) &&
        pdf_ps_obj_has_type(&s->cur[-2], PDF_PS_OBJ_ARRAY) &&
        pdf_ps_obj_has_type(&s->cur[-1], PDF_PS_OBJ_INTEGER) &&
        pdf_ps_obj_has_type(&s->cur[0], PDF_PS_OBJ_NAME)) {
        if (s->cur[-1].val.i < s->cur[-2].size) {
            pdf_ps_make_name(&s->cur[-2].val.arr[s->cur[-1].val.i], s->cur[0].val.name, s->cur[0].size);
        }
    }