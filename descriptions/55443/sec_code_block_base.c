gs_main_run_string(minst,
                "$error /dstack undef \
                 $error /estack undef \
                 $error /ostack undef",
                 0 , &exit_code, &error_object);

            ref_stack_clear(&o_stack);
            code = interp_reclaim(&minst->i_ctx_p, avm_global);

            /* We ignore gs_error_VMerror because it comes from gs_vmreclaim()
            calling context_state_load(), and we don't seem to depend on the
            missing fields. */
            if (code == gs_error_VMerror) {
                if (exit_status == 0 || exit_status == gs_error_Quit) {
                    exit_status = gs_error_VMerror;
                }
            }
            else if (code < 0) {
                ref error_name;
                if (tempnames)
                    free(tempnames);

                if (gs_errorname(i_ctx_p, code, &error_name) >= 0) {
                    char err_str[32] = {0};
                    name_string_ref(imemory, &error_name, &error_name);
                    memcpy(err_str, error_name.value.const_bytes, r_size(&error_name));
                    emprintf2(imemory, "ERROR: %s (%d) reclaiming the memory while the interpreter finalization.\n", err_str, code);
                }
                else {
                    emprintf1(imemory, "UNKNOWN ERROR %d reclaiming the memory while the interpreter finalization.\n", code);
                }
#ifdef MEMENTO
                if (Memento_squeezing() && code != gs_error_VMerror ) return gs_error_Fatal;
#endif
                return gs_error_Fatal;
            }
            i_ctx_p = minst->i_ctx_p; /* interp_reclaim could change it. */