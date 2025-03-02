unsigned int n, k;

    env.process_blend ();
    k = env.get_region_count ();
    n = env.argStack.pop_uint ();
    /* copy the blend values into blend array of the default values */
    unsigned int start = env.argStack.get_count () - ((k+1) * n);
    for (unsigned int i = 0; i < n; i++)
      env.argStack[start + i].set_blends (n, i, k, &env.argStack[start + n + (i * k)]);

    /* pop off blend values leaving default values now adorned with blend values */
    env.argStack.pop (k * n);