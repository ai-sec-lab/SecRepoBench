int
gs_main_init_with_args01(gs_main_instance * minst, int argc, char *argv[])
{
    const char *arg;
    arg_list args;
    int code;
    int have_dumped_args = 0;

    /* Now we actually process them */
    code = arg_init(&args, (const char **)argv, argc,
                    gs_main_arg_fopen, (void *)minst,
                    minst->get_codepoint,
                    minst->heap);
    if (code < 0)
        return code;
    code = gs_main_init0(minst, 0, 0, 0, GS_MAX_LIB_DIRS);
    if (code < 0)
        return code;
/* This first check is not needed on VMS since GS_LIB evaluates to the same
   value as that returned by gs_lib_default_path.  Also, since GS_LIB is
   defined as a searchlist logical and getenv only returns the first entry
   in the searchlist, it really doesn't make sense to search that particular
   directory twice.
*/
#ifndef __VMS
    {
        int len = 0;
        int code = gp_getenv(GS_LIB, (char *)0, &len);

        if (code < 0) {         /* key present, value doesn't fit */
            char *path = (char *)gs_alloc_bytes(minst->heap, len, "GS_LIB");

            gp_getenv(GS_LIB, path, &len);      /* can't fail */
            minst->lib_path.env = path;
        }
    }
#endif /* __VMS */
    minst->lib_path.final = gs_lib_default_path;
    code = gs_main_set_lib_paths(minst);
    if (code < 0)
        return code;
    /* Prescan the command line for --help and --version. */
    {
        int i;
        bool helping = false;

        for (i = 1; i < argc; ++i)
            if (!arg_strcmp(&args, argv[i], "--")) {
                /* A PostScript program will be interpreting all the */
                /* remaining switches, so stop scanning. */
                helping = false;
                break;
            } else if (!arg_strcmp(&args, argv[i], "--help")) {
                print_help(minst);
                helping = true;
            } else if (!arg_strcmp(&args, argv[i], "--debug")) {
                gs_debug_flags_list(minst->heap);
                helping = true;
            } else if (!arg_strcmp(&args, argv[i], "--version")) {
                print_version(minst);
                puts(minst->heap, "");  /* \n */
                helping = true;
            }
        if (helping)
            return gs_error_Info;
    }
    /* Execute files named in the command line, */
    /* processing options along the way. */
    /* Wait until the first file name (or the end */
    /* of the line) to finish initialization. */
    minst->run_start = true;

    {
        int len = 0;
        int code = gp_getenv(GS_OPTIONS, (char *)0, &len);

        if (code < 0) {         /* key present, value doesn't fit */
            char *opts =
            (char *)gs_alloc_bytes(minst->heap, len, "GS_OPTIONS");

            gp_getenv(GS_OPTIONS, opts, &len);  /* can't fail */
            if (arg_push_decoded_memory_string(&args, opts, false, true, minst->heap))
                return gs_error_Fatal;
        }
    }
    while ((code = arg_next(&args, (const char **)&arg, minst->heap)) > 0) {
        code = gs_lib_ctx_stash_sanitized_arg(minst->heap->gs_lib_ctx, arg);
        if (code < 0)
            return code;
        switch (*arg) {
            case '-':
                code = swproc(minst, arg, &args);
                if (code < 0)
                    return code;
                if (code > 0)
                    outprintf(minst->heap, "Unknown switch %s - ignoring\n", arg);
                if (gs_debug[':'] && !have_dumped_args) {
                    int i;

                    dmprintf1(minst->heap, "%% Args passed to instance 0x%p: ", minst);
                    for (i=1; i<argc; i++)
                        dmprintf1(minst->heap, "%s ", argv[i]);
                    dmprintf(minst->heap, "\n");
                    have_dumped_args = 1;
                }
                break;
            default:
                /* default is to treat this as a file name to be run */
                code = argproc(minst, arg);
                if (code < 0)
                    return code;
                if (minst->saved_pages_test_mode) {
                    gx_device *pdev;
                    int ret;
                    gxdso_device_child_request child_dev_data;

                    /* get the real target (printer) device */
                    pdev = gs_currentdevice(minst->i_ctx_p->pgs);
                    do {
                        child_dev_data.target = pdev;
                        ret = dev_proc(pdev, dev_spec_op)(pdev, gxdso_device_child, &child_dev_data,
                                                          sizeof(child_dev_data));
                        if (ret > 0)
                            pdev = child_dev_data.target;
                    } while ((ret > 0) && (child_dev_data.n != 0));
                    if ((code = gx_saved_pages_param_process((gx_device_printer *)pdev,
                               (byte *)"print normal flush", 18)) < 0)
                        return code;
                    if (code > 0)
                        if ((code = gs_erasepage(minst->i_ctx_p->pgs)) < 0)
                            return code;
                }
        }
    }

    return code;
}