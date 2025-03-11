unsigned int n, k;

    interp_env.process_blend ();
    k = interp_env.get_region_count ();
    n = interp_env.argStack.pop_uint ();
    /* copy the blend values into blend array of the default values */
    unsigned int start = interp_env.argStack.get_count () - ((k+1) * n);
    for (unsigned int i = 0; i < n; i++)
      interp_env.argStack[start + i].set_blends (n, i, k, &interp_env.argStack[start + n + (i * k)]);

    /* pop off blend values leaving default values now adorned with blend values */
    interp_env.argStack.pop (k * n);