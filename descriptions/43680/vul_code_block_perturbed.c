if (nstack != NULL) {
                memcpy(nstack, s->stack, (currsize - 1) * sizeof(pdf_ps_stack_object_t));

                for (index = 0; index < PDF_PS_STACK_GUARDS; index++)
                    nstack[newsize - PDF_PS_STACK_GUARDS + index].type = PDF_PS_OBJ_STACK_TOP;

                for (index = currsize - 2; index < newsize - PDF_PS_STACK_GUARDS; index++) {
                    pdf_ps_make_null(&(nstack[index]));
                }

                gs_free_object(s->pdfi_ctx->memory, s->stack, "pdf_ps_stack_push(s->stack)");
                s->stack = nstack;
                s->cur = s->stack + currsize - 1;
                s->toplim = s->stack + newsize - PDF_PS_STACK_GROW_SIZE;
            }