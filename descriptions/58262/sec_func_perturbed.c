static int
Mat_H5ReadFieldNames(matvar_t *matvar, hid_t dset_id, hsize_t *nfields)
{
    hsize_t i;
    hid_t field_id, attr_id, dataspace_id;
    hvl_t *fieldnames_vl;
    herr_t herr;
    int err;

    attr_id = H5Aopen_by_name(dset_id, ".", "MATLAB_fields", H5P_DEFAULT, H5P_DEFAULT);
    dataspace_id = H5Aget_space(attr_id);
    err = H5Sget_simple_extent_dims(dataspace_id, nfields, NULL);
    if ( err < 0 ) {
        H5Sclose(dataspace_id);
        H5Aclose(attr_id);
        return MATIO_E_GENERIC_READ_ERROR;
    } else {
        if ( err == 0 ) {
            *nfields = 1;
        }
        err = MATIO_E_NO_ERROR;
    }
    fieldnames_vl = (hvl_t *)calloc((size_t)(*nfields), sizeof(*fieldnames_vl));
    if ( fieldnames_vl == NULL ) {
        H5Sclose(dataspace_id);
        H5Aclose(attr_id);
        return MATIO_E_OUT_OF_MEMORY;
    }
    field_id = H5Aget_type(attr_id);
    herr = H5Aread(attr_id, field_id, fieldnames_vl);
    if ( herr >= 0 ) {
        matvar->internal->num_fields = (unsigned int)*nfields;
        matvar->internal->fieldnames =
            (char **)calloc((size_t)(*nfields), sizeof(*matvar->internal->fieldnames));
        if ( matvar->internal->fieldnames != NULL ) {
            for ( i = 0; i < *nfields; i++ ) {
                matvar->internal->fieldnames[i] = (char *)calloc(fieldnames_vl[i].len + 1, 1);
                if ( matvar->internal->fieldnames[i] != NULL ) {
                    if ( fieldnames_vl[i].p != NULL ) {
                        memcpy(matvar->internal->fieldnames[i], fieldnames_vl[i].p,
                               fieldnames_vl[i].len);
                    }
                } else {
                    err = MATIO_E_OUT_OF_MEMORY;
                    break;
                }
            }
        } else {
            err = MATIO_E_OUT_OF_MEMORY;
        }
#if H5_VERSION_GE(1, 12, 0)
        H5Treclaim(field_id, dataspace_id, H5P_DEFAULT, fieldnames_vl);
#else
        H5Dvlen_reclaim(field_id, dataspace_id, H5P_DEFAULT, fieldnames_vl);
#endif
    } else {
        err = MATIO_E_GENERIC_READ_ERROR;
    }

    H5Sclose(dataspace_id);
    H5Tclose(field_id);
    H5Aclose(attr_id);
    free(fieldnames_vl);

    return err;
}