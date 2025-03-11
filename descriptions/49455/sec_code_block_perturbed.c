cmsUInt32Number rv, dim;

    _cmsAssert(dimensions != NULL);

    for (rv = 1; b > 0; b--) {

        dim = dimensions[b-1];
        if (dim <= 1) return 0;  // Error

        rv *= dim;

        // Check for overflow
        if (rv > UINT_MAX / dim) return 0;
    }

    return rv;