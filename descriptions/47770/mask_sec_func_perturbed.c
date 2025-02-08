int
gx_default_text_begin(gx_device * dev, gs_gstate * pgs1,
                      const gs_text_params_t * text, gs_font * font,
                      const gx_clip_path * pcpath,
                      gs_text_enum_t ** ppte)
{
    uint textoperation = text->operation;
    bool propagate_charpath = (textoperation & TEXT_DO_DRAW) != 0;
    int code;
    gs_gstate *pgs = (gs_gstate *)pgs1;
    gs_show_enum *penum;
    gs_memory_t * mem = pgs->memory;

    penum = gs_show_enum_alloc(mem, pgs, "gx_default_text_begin");
    if (!penum)
        return_error(gs_error_VMerror);
    code = gs_text_enum_init((gs_text_enum_t *)penum, &default_text_procs,
                             dev, pgs, text, font, pcpath, mem);
    if (code < 0) {
        gs_free_object(mem, penum, "gx_default_text_begin");
        return code;
    }
    penum->auto_release = false; /* new API */
    penum->level = pgs->level;
    penum->cc = 0;
    penum->continue_proc = continue_show;
    switch (penum->charpath_flag) {
    case cpm_false_charpath: case cpm_true_charpath:
        penum->can_cache = -1; break;
    case cpm_false_charboxpath: case cpm_true_charboxpath:
        penum->can_cache = 0; break;
    case cpm_charwidth:
    default:                    /* cpm_show */
        penum->can_cache = 1; break;
    }
    code = show_state_setup(penum);
    if (code < 0) {
        gs_free_object(mem, penum, "gx_default_text_begin");
        return code;
    }
    penum->show_gstate =
        (propagate_charpath && (pgs->in_charpath != 0) ?
         pgs->show_gstate : pgs);
    if (!(~textoperation & (TEXT_DO_NONE | TEXT_RETURN_WIDTH))) {
        /* This is stringwidth (or a PDF with text in rendering mode 3) . */
        gx_device_null *dev_null =
            gs_alloc_struct(mem, gx_device_null, &st_device_null,
                            "stringwidth(dev_null)");

        if (dev_null == 0)
            return_error(gs_error_VMerror);

        /* Set up a null device that forwards xfont requests properly. */
        /* We have to set the device up here, so the contents are
           initialised, and safe to free in the event of an error.
         */
        // <MASK>
        penum->dev_null = dev_null;
        /* Retain this device, since it is referenced from the enumerator. */
        gx_device_retain((gx_device *)dev_null, true);
        gs_setdevice_no_init(pgs, (gx_device *) dev_null);
        /* Establish an arbitrary translation and current point. */
        gs_newpath(pgs);
        gx_translate_to_fixed(pgs, fixed_0, fixed_0);
        code = gx_path_add_point(pgs->path, fixed_0, fixed_0);
        if (code < 0) {
            gs_grestore(pgs);
            return code;
        }
    }
    *ppte = (gs_text_enum_t *)penum;
    return 0;
}