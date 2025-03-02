herr_t herr;
    int err, ndims;

    attr_id = H5Aopen_by_name(dset_id, ".", "MATLAB_fields", H5P_DEFAULT, H5P_DEFAULT);
    space_id = H5Aget_space(attr_id);
    ndims = H5Sget_simple_extent_ndims(space_id);
    if ( 0 > ndims || 1 < ndims ) {
        *nfields = 0;
        H5Sclose(space_id);
        H5Aclose(attr_id);
        return MATIO_E_GENERIC_READ_ERROR;
    } else {
        err = MATIO_E_NO_ERROR;
    }
    (void)H5Sget_simple_extent_dims(space_id, nfields, NULL);