static int
Mat_H5ReadFieldNames(matvar_t *matvar, hid_t dsethandle, hsize_t *nfields)
{
    hsize_t i;
    hid_t field_id, attr_id, space_id;
    hvl_t *fieldnames_vl;
    herr_t herr;
    // <MASK>
    if ( fieldnames_vl == NULL ) {
        H5Sclose(space_id);
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
        H5Treclaim(field_id, space_id, H5P_DEFAULT, fieldnames_vl);
#else
        H5Dvlen_reclaim(field_id, space_id, H5P_DEFAULT, fieldnames_vl);
#endif
    } else {
        err = MATIO_E_GENERIC_READ_ERROR;
    }

    H5Sclose(space_id);
    H5Tclose(field_id);
    H5Aclose(attr_id);
    free(fieldnames_vl);

    return err;
}