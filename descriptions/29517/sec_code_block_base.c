/* Check if the variable is sparse */
    if ( H5Aexists_by_name(dset_id,".","MATLAB_sparse",H5P_DEFAULT) ) {
        herr_t herr;
        hid_t sparse_dset_id;
        unsigned nrows = 0;

        attr_id = H5Aopen_by_name(dset_id,".","MATLAB_sparse",H5P_DEFAULT,H5P_DEFAULT);
        herr = H5Aread(attr_id,H5T_NATIVE_UINT,&nrows);
        H5Aclose(attr_id);
        if ( herr < 0 ) {
            return MATIO_E_GENERIC_READ_ERROR;
        }

        matvar->class_type = MAT_C_SPARSE;

        sparse_dset_id = H5Dopen(dset_id,"jc",H5P_DEFAULT);
        matvar->dims = Mat_H5ReadDims(sparse_dset_id, &nelems, &matvar->rank);
        H5Dclose(sparse_dset_id);
        if ( NULL != matvar->dims ) {
            if ( 1 == matvar->rank ) {
                size_t* dims = (size_t*)realloc(matvar->dims, 2*sizeof(*matvar->dims));
                if ( NULL != dims ) {
                    matvar->rank = 2;
                    matvar->dims = dims;
                }
            }
            if ( 2 == matvar->rank ) {
                matvar->dims[1] = matvar->dims[0] - 1;
                matvar->dims[0] = nrows;
            }
        } else {
            return MATIO_E_UNKNOWN_ERROR;
        }

        /* Test if dataset type is compound and if so if it's complex */
        if ( H5Lexists(dset_id,"data",H5P_DEFAULT) ) {
            hid_t type_id;
            sparse_dset_id = H5Dopen(dset_id,"data",H5P_DEFAULT);
            type_id = H5Dget_type(sparse_dset_id);
            if ( H5T_COMPOUND == H5Tget_class(type_id) ) {
                /* FIXME: Any more checks? */
                matvar->isComplex = MAT_F_COMPLEX;
            }
            H5Tclose(type_id);
            H5Dclose(sparse_dset_id);
        }
        return MATIO_E_NO_ERROR;
    }

    if (MAT_C_STRUCT != matvar->class_type ) {
        return MATIO_E_GENERIC_READ_ERROR;
    }

    /* Check if the structure defines its fields in MATLAB_fields */
    if ( H5Aexists_by_name(dset_id,".","MATLAB_fields",H5P_DEFAULT) ) {
        err = Mat_H5ReadFieldNames(matvar, dset_id, &nfields);
        if ( err ) {
            return err;
        }
    } else {
        H5G_info_t group_info;
        matvar->internal->num_fields = 0;
        group_info.nlinks = 0;
        H5Gget_info(dset_id, &group_info);
        if ( group_info.nlinks > 0 ) {
            struct ReadGroupInfoIterData group_data = {0, NULL};
            herr_t herr;

            /* First iteration to retrieve number of relevant links */
            herr = H5Literate_by_name(dset_id, matvar->internal->hdf5_name, H5_INDEX_NAME,
                H5_ITER_NATIVE, NULL, Mat_H5ReadGroupInfoIterate,
                (void *)&group_data, H5P_DEFAULT);
            if ( herr > 0 && group_data.nfields > 0 ) {
                matvar->internal->fieldnames =
                    (char**)calloc((size_t)(group_data.nfields),sizeof(*matvar->internal->fieldnames));
                group_data.nfields = 0;
                group_data.matvar = matvar;
                if ( matvar->internal->fieldnames != NULL ) {
                    /* Second iteration to fill fieldnames */
                    H5Literate_by_name(dset_id, matvar->internal->hdf5_name, H5_INDEX_NAME,
                        H5_ITER_NATIVE, NULL, Mat_H5ReadGroupInfoIterate,
                        (void *)&group_data, H5P_DEFAULT);
                }
                matvar->internal->num_fields = (unsigned)group_data.nfields;
                nfields = group_data.nfields;
            }
        }
    }