pdfi_push(pdfcontext, (pdf_obj *)stream_obj);
    pdfi_countdown(stream_obj); /* get rid of extra ref */

    stream_obj->stream_dict->indirect_num = stream_obj->stream_dict->object_num = objnum;
    stream_obj->stream_dict->indirect_gen = stream_obj->stream_dict->generation_num = gen;
    stream_obj->stream_offset = offset;

    /* This code may be a performance overhead, it simply skips over the stream contents
     * and checks that the stream ends with a 'endstream endobj' pair. We could add a
     * 'go faster' flag for users who are certain their PDF files are well-formed. This
     * could also allow us to skip all kinds of other checking.....
     */

    code = pdfi_dict_get_int(pdfcontext, (pdf_dict *)stream_obj->stream_dict, "Length", &i);
    if (code < 0) {
        char extra_info[gp_file_name_sizeof];

        gs_sprintf(extra_info, "Stream object %u missing mandatory keyword /Length, unable to verify the stream length.\n", objnum);
        pdfi_set_error(pdfcontext, 0, NULL, E_PDF_BADSTREAM, "pdfi_read_stream_object", extra_info);
        return 0;
    }

    if (i < 0 || (i + offset)> pdfcontext->main_stream_length) {
        char extra_info[gp_file_name_sizeof];

        gs_sprintf(extra_info, "Stream object %u has /Length which, when added to offset of object, exceeds file size.\n", objnum);
        pdfi_set_error(pdfcontext, 0, NULL, E_PDF_BADSTREAM, "pdfi_read_stream_object", extra_info);
    } else {
        code = pdfi_seek(pdfcontext, pdfcontext->main_stream, i, SEEK_CUR);
        if (code < 0) {
            pdfi_pop(pdfcontext, 1);
            return code;
        }

        stream_obj->Length = 0;
        stream_obj->length_valid = false;

        code = pdfi_read_token(pdfcontext, pdfcontext->main_stream, objnum, gen);
        if (code < 0 || pdfi_count_stack(pdfcontext) < 2) {
            char extra_info[gp_file_name_sizeof];

            gs_sprintf(extra_info, "Failed to find a valid object at end of stream object %u.\n", objnum);
            pdfi_log_info(pdfcontext, "pdfi_read_stream_object", extra_info);
            /* It is possible for pdfi_read_token to clear the stack, losing the stream object. If that
             * happens give up.
             */
            if (pdfi_count_stack(pdfcontext) == 0)
                return code;
        }
        else {
            if (((pdf_obj *)pdfcontext->stack_top[-1])->type != PDF_KEYWORD) {
                char extra_info[gp_file_name_sizeof];

                gs_sprintf(extra_info, "Failed to find 'endstream' keyword at end of stream object %u.\n", objnum);
                pdfi_set_error(pdfcontext, 0, NULL, E_PDF_MISSINGENDOBJ, "pdfi_read_stream_object", extra_info);
            } else {
                keyword = ((pdf_keyword *)pdfcontext->stack_top[-1]);
                if (keyword->key != TOKEN_ENDSTREAM) {
                    char extra_info[gp_file_name_sizeof];

                    gs_sprintf(extra_info, "Stream object %u has an incorrect /Length of %"PRIu64"\n", objnum, i);
                    pdfi_log_info(pdfcontext, "pdfi_read_stream_object", extra_info);
                } else {
                    /* Cache the Length in the stream object and mark it valid */
                    stream_obj->Length = i;
                    stream_obj->length_valid = true;
                }
            }
            pdfi_pop(pdfcontext, 1);
        }
    }

    /* If we failed to find a valid object, or the object wasn't a keyword, or the
     * keywrod wasn't 'endstream' then the Length is wrong. We need to have the correct
     * Length for streams if we have encrypted files, because we must install a
     * SubFileDecode filter iwth a Length (EODString is incompatible with AES encryption)
     * Rather than mess about checking for encryption, we'll choose to just correctly
     * calculate the Length of all streams. Although this takes time, it will only
     * happen for files which are invalid.
     */
    if (stream_obj->length_valid != true) {
        char Buffer[10];
        unsigned int loop, bytes, total = 0;

        code = pdfi_seek(pdfcontext, pdfcontext->main_stream, stream_obj->stream_offset, SEEK_SET);
        if (code < 0) {
            pdfi_pop(pdfcontext, 1);
            return code;
        }
        memset(Buffer, 0x00, 10);
        bytes = pdfi_read_bytes(pdfcontext, (byte *)Buffer, 1, 9, pdfcontext->main_stream);
        if (bytes < 9)
            return_error(gs_error_ioerror);

        total = bytes;
        do {
            if (memcmp(Buffer, "endstream", 9) == 0) {
                stream_obj->Length = total - 9;
                stream_obj->length_valid = true;
                break;
            }
            if (memcmp(Buffer, "endobj", 6) == 0) {
                stream_obj->Length = total - 6;
                stream_obj->length_valid = true;
                break;
            }
            for (loop = 0;loop < 9;loop++){
                Buffer[loop] = Buffer[loop + 1];
            }
            bytes = pdfi_read_bytes(pdfcontext, (byte *)&Buffer[9], 1, 1, pdfcontext->main_stream);
            total += bytes;
        } while(bytes);
        if (bytes <= 0)
            return_error(gs_error_ioerror);
        return 0;
    }

    code = pdfi_read_token(pdfcontext, pdfcontext->main_stream, objnum, gen);
    if (code < 0) {
        if (pdfcontext->args.pdfstoponerror)
            return code;
        else
            /* Something went wrong looking for endobj, but we found endstream, so assume
             * for now that will suffice.
             */
            pdfi_set_error(pdfcontext, 0, NULL, E_PDF_MISSINGENDOBJ, "pdfi_read_stream_object", NULL);
        return 0;
    }

    if (pdfi_count_stack(pdfcontext) < 2)
        return_error(gs_error_stackunderflow);

    if (((pdf_obj *)pdfcontext->stack_top[-1])->type != PDF_KEYWORD) {
        pdfi_pop(pdfcontext, 1);
        if (pdfcontext->args.pdfstoponerror)
            return_error(gs_error_typecheck);
        pdfi_set_error(pdfcontext, 0, NULL, E_PDF_MISSINGENDOBJ, "pdfi_read_stream_object", NULL);
        /* Didn't find an endobj, but we have an endstream, so assume
         * for now that will suffice
         */
        return 0;
    