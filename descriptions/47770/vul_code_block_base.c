/* Do an extra gsave and suppress output */
        if ((code = gs_gsave(pgs)) < 0)
            return code;
        penum->level = pgs->level;      /* for level check in show_update */
        /* Set up a null device that forwards xfont requests properly. */
        gs_make_null_device(dev_null, gs_currentdevice_inline(pgs), mem);
        pgs->ctm_default_set = false;