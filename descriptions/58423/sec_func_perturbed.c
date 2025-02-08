static int pdfi_check_Font(pdf_context *pdfctx, pdf_dict *font, pdf_dict *page_dict,
                           pdfi_check_tracker_t *tracker)
{
    int code = 0;
    pdf_obj *o = NULL;

    if (resource_is_checked(tracker, (pdf_obj *)font))
        return 0;

    if (pdfi_type_of(font) != PDF_DICT)
        return_error(gs_error_typecheck);

    if (tracker->font_array != NULL) {
        /* If we get to here this is a font we have not seen before. We need
         * to make a new font array big enough to hold the existing entries +1
         * copy the existing entries to the new array and free the old array.
         * Finally create a dictionary with all the font information we want
         * and add it to the array.
         */
        pdf_array *new_fonts = NULL;
        int index = 0;
        pdf_obj *array_obj = NULL;
        pdf_dict *font_info_dict = NULL;

        /* Let's start by gathering the information we need and storing it in a dictionary */
        code = pdfi_dict_alloc(pdfctx, 4, &font_info_dict);
        if (code < 0)
            return code;
        pdfi_countup(font_info_dict);

        if (font->object_num != 0) {
            pdf_num *int_obj = NULL;

            code = pdfi_object_alloc(pdfctx, PDF_INT, 0, (pdf_obj **)&int_obj);
            if (code >= 0) {
                pdfi_countup(int_obj);
                int_obj->value.i = font->object_num;
                code = pdfi_dict_put(pdfctx, font_info_dict, "ObjectNum", (pdf_obj *)int_obj);
                pdfi_countdown(int_obj);
            }
            if (code < 0) {
                pdfi_countdown(font_info_dict);
                return code;
            }
        }

        code = pdfi_dict_get(pdfctx, font, "BaseFont", &array_obj);
        if (code >= 0) {
            code = pdfi_dict_put(pdfctx, font_info_dict, "BaseFont", array_obj);
            if (code < 0) {
                pdfi_countdown(array_obj);
                pdfi_countdown(font_info_dict);
                return code;
            }
        }
        pdfi_countdown(array_obj);
        array_obj = NULL;

        code = pdfi_dict_get(pdfctx, font, "ToUnicode", &array_obj);
        if (code >= 0)
            code = pdfi_dict_put(pdfctx, font_info_dict, "ToUnicode", PDF_TRUE_OBJ);
        else
            code = pdfi_dict_put(pdfctx, font_info_dict, "ToUnicode", PDF_FALSE_OBJ);
        pdfi_countdown(array_obj);
        array_obj = NULL;
        if (code < 0)
            return code;

        code = pdfi_dict_get(pdfctx, font, "FontDescriptor", &array_obj);
        if (code >= 0) {
            bool known = false;

            (void)pdfi_dict_known(pdfctx, (pdf_dict *)array_obj, "FontFile", &known);
            if (!known) {
                (void)pdfi_dict_known(pdfctx, (pdf_dict *)array_obj, "FontFile2", &known);
                if (!known) {
                    (void)pdfi_dict_known(pdfctx, (pdf_dict *)array_obj, "FontFile3", &known);
                }
            }

            if (known > 0)
                code = pdfi_dict_put(pdfctx, font_info_dict, "Embedded", PDF_TRUE_OBJ);
            else
                code = pdfi_dict_put(pdfctx, font_info_dict, "Embedded", PDF_FALSE_OBJ);
        } else
            code = pdfi_dict_put(pdfctx, font_info_dict, "Embedded", PDF_FALSE_OBJ);

        pdfi_countdown(array_obj);
        array_obj = NULL;

        if (code < 0)
            return code;


        code = pdfi_dict_knownget_type(pdfctx, font, "Subtype", PDF_NAME, &array_obj);
        if (code > 0) {
            code = pdfi_dict_put(pdfctx, font_info_dict, "Subtype", array_obj);
            if (code < 0) {
                pdfi_countdown(array_obj);
                pdfi_countdown(font_info_dict);
                return code;
            }

            if (pdfi_name_is((pdf_name *)array_obj, "Type3")) {
                pdfi_countdown(o);
                o = NULL;

                code = pdfi_dict_knownget_type(pdfctx, font, "Resources", PDF_DICT, &o);
                if (code > 0)
                    (void)pdfi_check_Resources(pdfctx, (pdf_dict *)o, page_dict, tracker);
            }

            if (pdfi_name_is((const pdf_name *)array_obj, "Type0")){
                pdf_array *descendants = NULL;
                pdf_dict *desc_font = NULL;

                code = pdfi_dict_get(pdfctx, font, "DescendantFonts", (pdf_obj **)&descendants);
                if (code >= 0) {
                    code = pdfi_array_get(pdfctx, descendants, 0, (pdf_obj **)&desc_font);
                    if (code >= 0){
                        pdf_array *desc_array = NULL;

                        code = pdfi_array_alloc(pdfctx, 0, &desc_array);
                        pdfi_countup(desc_array);
                        if (code >= 0) {
                            pdf_array *saved = tracker->font_array;

                            tracker->font_array = desc_array;
                            (void)pdfi_check_Font(pdfctx, desc_font, page_dict, tracker);
                            (void)pdfi_dict_put(pdfctx, font_info_dict, "Descendants", (pdf_obj *)tracker->font_array);
                            pdfi_countdown((pdf_obj *)tracker->font_array);
                            tracker->font_array = saved;
                        }
                        pdfi_countdown(descendants);
                        pdfi_countdown(desc_font);
                    }
                }
            }
        }
        pdfi_countdown(array_obj);
        array_obj = NULL;

        code = pdfi_array_alloc(pdfctx, pdfi_array_size(tracker->font_array) + 1, &new_fonts);
        if (code < 0) {
            pdfi_countdown(font_info_dict);
            return code;
        }
        pdfi_countup(new_fonts);

        for (index = 0; index < pdfi_array_size(tracker->font_array); index++) {
            code = pdfi_array_get(pdfctx, tracker->font_array, index, &array_obj);
            if (code < 0) {
                pdfi_countdown(font_info_dict);
                pdfi_countdown(new_fonts);
                return code;
            }
            code = pdfi_array_put(pdfctx, new_fonts, index, array_obj);
            pdfi_countdown(array_obj);
            if (code < 0) {
                pdfi_countdown(font_info_dict);
                pdfi_countdown(new_fonts);
                return code;
            }
        }
        code = pdfi_array_put(pdfctx, new_fonts, index, (pdf_obj *)font_info_dict);
        if (code < 0) {
            pdfi_countdown(font_info_dict);
            pdfi_countdown(new_fonts);
            return code;
        }
        pdfi_countdown(font_info_dict);
        pdfi_countdown(tracker->font_array);
        tracker->font_array = new_fonts;
    } else {
        code = pdfi_dict_knownget_type(pdfctx, font, "Subtype", PDF_NAME, &o);
        if (code > 0) {
            if (pdfi_name_is((pdf_name *)o, "Type3")) {
                pdfi_countdown(o);
                o = NULL;

                code = pdfi_dict_knownget_type(pdfctx, font, "Resources", PDF_DICT, &o);
                if (code > 0)
                    (void)pdfi_check_Resources(pdfctx, (pdf_dict *)o, page_dict, tracker);
            }
        }

        pdfi_countdown(o);
        o = NULL;
    }

    return 0;
}