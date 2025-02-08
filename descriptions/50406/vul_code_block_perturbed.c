if (pdfi_type_of(Parent) != PDF_DICT) {
                if (pdfi_type_of(Parent) == PDF_INDIRECT) {
                    pdf_indirect_ref *o = (pdf_indirect_ref *)Parent;

                    code = pdfi_dereference(ctx, o->ref_object_num, o->ref_generation_num, (pdf_obj **)&Parent);
                    pdfi_countdown(o);
                    if (code < 0)
                        break;
                } else {
                    pdfi_countdown(Parent);
                    break;
                }
            }
            pdfi_countdown(currdict);
            currdict = Parent;
            pdfi_countup(currdict);