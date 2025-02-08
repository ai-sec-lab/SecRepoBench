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