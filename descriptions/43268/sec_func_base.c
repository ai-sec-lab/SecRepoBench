int pdfi_dereference(pdf_context *ctx, uint64_t obj, uint64_t gen, pdf_obj **object)
{
    xref_entry *entry;
    int code, stack_depth = pdfi_count_stack(ctx);
    gs_offset_t saved_stream_offset;
    bool saved_decrypt_strings = ctx->encryption.decrypt_strings;

    *object = NULL;

    if (ctx->xref_table == NULL)
        return_error(gs_error_typecheck);

    if (obj >= ctx->xref_table->xref_size) {
        char extra_info[gp_file_name_sizeof];

        gs_sprintf(extra_info, "Error, attempted to dereference object %"PRIu64", which is not present in the xref table\n", obj);
        pdfi_set_error(ctx, 0, NULL, E_PDF_BADOBJNUMBER, "pdfi_dereference", extra_info);

        if(ctx->args.pdfstoponerror)
            return_error(gs_error_rangecheck);

        code = pdfi_object_alloc(ctx, PDF_NULL, 0, object);
        if (code == 0)
            pdfi_countup(*object);
        return code;
    }

    entry = &ctx->xref_table->xref[obj];

    if(entry->object_num == 0)
        return_error(gs_error_undefined);

    if (entry->free) {
        char extra_info[gp_file_name_sizeof];

        gs_sprintf(extra_info, "Attempt to dereference free object %"PRIu64", trying next object number as offset.\n", entry->object_num);
        pdfi_set_error(ctx, 0, NULL, E_PDF_DEREF_FREE_OBJ, "pdfi_dereference", extra_info);
    }

    if (ctx->loop_detection) {
        if (pdfi_loop_detector_check_object(ctx, obj) == true)
            return_error(gs_error_circular_reference);
        if (entry->free) {
            code = pdfi_loop_detector_add_object(ctx, obj);
            if (code < 0)
                return code;
        }
    }
    if (entry->cache != NULL){
        pdf_obj_cache_entry *cache_entry = entry->cache;

#if CACHE_STATISTICS
        ctx->hits++;
#endif
        *object = cache_entry->o;
        pdfi_countup(*object);

        pdfi_promote_cache_entry(ctx, cache_entry);
    } else {
        saved_stream_offset = pdfi_unread_tell(ctx);

        if (entry->compressed) {
            /* This is an object in a compressed object stream */
            ctx->encryption.decrypt_strings = false;

            code = pdfi_deref_compressed(ctx, obj, gen, object, entry);
            if (code < 0 || *object == NULL)
                goto error;
        } else {
            pdf_c_stream *SubFile_stream = NULL;
            pdf_string *EODString;
#if CACHE_STATISTICS
            ctx->misses++;
#endif
            ctx->encryption.decrypt_strings = true;

            code = pdfi_seek(ctx, ctx->main_stream, entry->u.uncompressed.offset, SEEK_SET);
            if (code < 0)
                goto error;

            code = pdfi_name_alloc(ctx, (byte *)"trailer", 6, (pdf_obj **)&EODString);
            if (code < 0)
                goto error;
            pdfi_countup(EODString);

            code = pdfi_apply_SubFileDecode_filter(ctx, 0, EODString, ctx->main_stream, &SubFile_stream, false);
            if (code < 0) {
                pdfi_countdown(EODString);
                goto error;
            }

            code = pdfi_read_object(ctx, SubFile_stream, entry->u.uncompressed.offset);

            /* pdfi_read_object() could do a repair, which would invalidate the xref and rebuild it.
             * reload the xref entry to be certain it is valid.
             */
            entry = &ctx->xref_table->xref[obj];

            pdfi_countdown(EODString);
            pdfi_close_file(ctx, SubFile_stream);
            if (code < 0) {
                int code1 = 0;
                if (entry->free) {
                    dmprintf2(ctx->memory, "Dereference of free object %"PRIu64", next object number as offset failed (code = %d), returning NULL object.\n", entry->object_num, code);
                    code = pdfi_object_alloc(ctx, PDF_NULL, 1, object);
                    if (code >= 0) {
                        pdfi_countup(*object);
                        goto free_obj;
                    }
                }
                ctx->encryption.decrypt_strings = saved_decrypt_strings;
                (void)pdfi_seek(ctx, ctx->main_stream, saved_stream_offset, SEEK_SET);
                pdfi_pop(ctx, pdfi_count_stack(ctx) - stack_depth);

                code1 = pdfi_repair_file(ctx);
                if (code1 == 0)
                    return pdfi_dereference(ctx, obj, gen, object);
                /* Repair failed, just give up and return an error */
                return code;
            }

            if (pdfi_count_stack(ctx) > 0 && (ctx->stack_top[-1])->object_num == obj) {
                *object = ctx->stack_top[-1];
                pdfi_countup(*object);
                pdfi_pop(ctx, 1);
                code = pdfi_add_to_cache(ctx, *object);
                if (code < 0) {
                    pdfi_countdown(*object);
                    goto error;
                }
            } else {
                pdfi_pop(ctx, 1);
                if (entry->free) {
                    dmprintf1(ctx->memory, "Dereference of free object %"PRIu64", next object number as offset failed, returning NULL object.\n", entry->object_num);
                    code = pdfi_object_alloc(ctx, PDF_NULL, 1, object);
                    if (code >= 0)
                        pdfi_countup(*object);
                    return code;
                }
                code = gs_note_error(gs_error_undefined);
                goto error;
            }
        }
free_obj:
        (void)pdfi_seek(ctx, ctx->main_stream, saved_stream_offset, SEEK_SET);
    }

    if (ctx->loop_detection && (*object)->object_num != 0) {
        code = pdfi_loop_detector_add_object(ctx, (*object)->object_num);
        if (code < 0) {
            ctx->encryption.decrypt_strings = saved_decrypt_strings;
            return code;
        }
    }
    ctx->encryption.decrypt_strings = saved_decrypt_strings;
    return 0;

error:
    ctx->encryption.decrypt_strings = saved_decrypt_strings;
    (void)pdfi_seek(ctx, ctx->main_stream, saved_stream_offset, SEEK_SET);
    /* Return the stack to the state at entry */
    pdfi_pop(ctx, pdfi_count_stack(ctx) - stack_depth);
    return code;
}