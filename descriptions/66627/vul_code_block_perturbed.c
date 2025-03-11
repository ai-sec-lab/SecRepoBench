herr_t herr;
    int err;

    attr_id = H5Aopen_by_name(dset_id, ".", "MATLAB_fields", H5P_DEFAULT, H5P_DEFAULT);
    dspace_id = H5Aget_space(attr_id);
    err = H5Sget_simple_extent_dims(dspace_id, nfields, NULL);
    if ( err < 0 ) {
        H5Sclose(dspace_id);
        H5Aclose(attr_id);
        return MATIO_E_GENERIC_READ_ERROR;
    } else {
        err = MATIO_E_NO_ERROR;
    }