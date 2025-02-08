/* Sanity check the offset and size of the CFF table and make sure the declared
         * size and position fits inside the data we have. Promote the 32-bit variables to
         * 64-bit to avoid overflow calculating the end of the table.
         */
        if (tableoffset == 0 || tlen == 0 || (uint64_t)tableoffset + (uint64_t)tlen > fbuflen) {
            gs_free_object(ctx->memory, pfbuf, "pdfi_read_cff_font(fbuf)");
            return_error(gs_error_invalidfont);
        }
        fbuf += tableoffset;
        fbuflen = tlen;