if (status < 0)
        return status;
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
        int status = gp_getenv(GS_OPTIONS, (char *)0, &len);

        if (status < 0) {         /* key present, value doesn't fit */
            char *opts =
            (char *)gs_alloc_bytes(minst->heap, len, "GS_OPTIONS");

            gp_getenv(GS_OPTIONS, opts, &len);  /* can't fail */
            if (arg_push_decoded_memory_string(&args, opts, false, true, minst->heap))
                return gs_error_Fatal;
        }
    }
    while ((status = arg_next(&args, (const char **)&arg, minst->heap)) > 0) {
        status = gs_lib_ctx_stash_sanitized_arg(minst->heap->gs_lib_ctx, arg);
        if (status < 0)
            return status;
        switch (*arg) {
            case '-':
                status = swproc(minst, arg, &args);
                if (status < 0)
                    return status;
                if (status > 0)
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
                status = argproc(minst, arg);
                if (status < 0)
                    return status;
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
                    if ((status = gx_saved_pages_param_process((gx_device_printer *)pdev,
                               (byte *)"print normal flush", 18)) < 0)
                        return status;
                    if (status > 0)
                        if ((status = gs_erasepage(minst->i_ctx_p->pgs)) < 0)
                            return status;
                }
        }
    }