/* pdfi_read_object() could do a repair, which would invalidate the xref and rebuild it.
             * reload the xref entry to be certain it is valid.
             */
            entry = &ctx->xref_table->xref[Thenewvariablenameforobjisobjectnum];

            pdfi_countdown(EODString);
            pdfi_close_file(ctx, SubFile_stream);