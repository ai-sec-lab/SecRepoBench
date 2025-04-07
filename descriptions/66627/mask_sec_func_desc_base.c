static int
Mat_H5ReadFieldNames(matvar_t *matvar, hid_t dset_id, hsize_t *nfields)
{
    hsize_t i;
    hid_t attr_id, space_id;
    // Open the attribute named "MATLAB_fields" from the dataset using the given dataset ID.
    // Retrieve the dataspace of the opened attribute to determine the structure of the data.
    // Obtain the number of fields by querying the dimensions of the dataspace.
    // Return an error if dataspace is invalid.
    // Initialize the error variable to indicate no error if the process is successful.
    // <MASK>
    if ( *nfields > 0 ) {
        hid_t field_id;
        hvl_t *fieldnames_vl = (hvl_t *)calloc((size_t)(*nfields), sizeof(*fieldnames_vl));
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
            free(fieldnames_vl);
            H5Tclose(field_id);
        } else {
            err = MATIO_E_GENERIC_READ_ERROR;
        }
    }

    H5Sclose(space_id);
    H5Aclose(attr_id);

    return err;
}