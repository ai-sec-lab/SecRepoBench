gs_make_null_device(dev_null, gs_currentdevice_inline(pgs), mem);

        /* Do an extra gsave and suppress output */
        if ((code = gs_gsave(pgs)) < 0) {
            gs_free_object(mem, dev_null, "gx_default_text_begin");
            return code;
        }
        penum->level = pgs->level;      /* for level check in show_update */
        pgs->ctm_default_set = false;