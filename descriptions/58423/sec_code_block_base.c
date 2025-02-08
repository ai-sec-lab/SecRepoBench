if (!known) {
                (void)pdfi_dict_known(ctx, (pdf_dict *)array_obj, "FontFile2", &known);
                if (!known) {
                    (void)pdfi_dict_known(ctx, (pdf_dict *)array_obj, "FontFile3", &known);
                }
            }

            if (known > 0)
                code = pdfi_dict_put(ctx, font_info_dict, "Embedded", PDF_TRUE_OBJ);
            else
                code = pdfi_dict_put(ctx, font_info_dict, "Embedded", PDF_FALSE_OBJ);