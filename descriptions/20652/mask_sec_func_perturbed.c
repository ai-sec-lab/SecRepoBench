int
gs_main_init_with_args01(gs_main_instance * minst, int argc, char *argv[])
{
    const char *arg;
    arg_list args;
    int status;
    int have_dumped_args = 0;

    /* Now we actually process them */
    status = arg_init(&args, (const char **)argv, argc,
                    gs_main_arg_fopen, (void *)minst,
                    minst->get_codepoint,
                    minst->heap);
    if (status < 0)
        return status;
    status = gs_main_init0(minst, 0, 0, 0, GS_MAX_LIB_DIRS);
    if (status < 0)
        return status;
/* This first check is not needed on VMS since GS_LIB evaluates to the same
   value as that returned by gs_lib_default_path.  Also, since GS_LIB is
   defined as a searchlist logical and getenv only returns the first entry
   in the searchlist, it really doesn't make sense to search that particular
   directory twice.
*/
#ifndef __VMS
    {
        int len = 0;
        int status = gp_getenv(GS_LIB, (char *)0, &len);

        if (status < 0) {         /* key present, value doesn't fit */
            char *path = (char *)gs_alloc_bytes(minst->heap, len, "GS_LIB");

            gp_getenv(GS_LIB, path, &len);      /* can't fail */
            minst->lib_path.env = path;
        }
    }
#endif /* __VMS */
    minst->lib_path.final = gs_lib_default_path;
    status = gs_main_set_lib_paths(minst);
    // <MASK>

    return status;
}