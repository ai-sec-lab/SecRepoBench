

        dim = Dims[b-1];
        if (dim <= 1) return 0;  // Error

        rv *= dim;

        // Check for overflow
        if (rv > UINT_MAX / dim) return 0;
    