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

    if ( nfields > 0 ) {
        H5O_INFO_T object_info;
        object_info.type = H5O_TYPE_UNKNOWN;
        H5OGET_INFO_BY_NAME(dset_id, matvar->internal->fieldnames[0], &object_info, H5P_DEFAULT);
        obj_type = object_info.type;
    } else {
        obj_type = H5O_TYPE_UNKNOWN;
    }
    if ( obj_type == H5O_TYPE_DATASET ) {
        hid_t field_type_id;
        field_id = H5Dopen(dset_id,matvar->internal->fieldnames[0],H5P_DEFAULT);
        field_type_id = H5Dget_type(field_id);
        if ( H5T_REFERENCE == H5Tget_class(field_type_id) ) {
            /* Check if the field has the MATLAB_class attribute. If so, it
             * means the structure is a scalar. Otherwise, the dimensions of
             * the field dataset is the dimensions of the structure
             */
            if ( H5Aexists_by_name(field_id,".","MATLAB_class",H5P_DEFAULT) ) {
                matvar->rank = 2;
                matvar->dims = (size_t*)malloc(2*sizeof(*matvar->dims));
                if ( NULL != matvar->dims ) {
                    matvar->dims[0] = 1;
                    matvar->dims[1] = 1;
                    nelems = 1;
                } else {
                    H5Tclose(field_type_id);
                    H5Dclose(field_id);
                    Mat_Critical("Error allocating memory for matvar->dims");
                    return MATIO_E_OUT_OF_MEMORY;
                }
            } else {
                matvar->dims = Mat_H5ReadDims(field_id, &nelems, &matvar->rank);
                if ( NULL != matvar->dims ) {
                    fields_are_variables = 0;
                } else {
                    H5Tclose(field_type_id);
                    H5Dclose(field_id);
                    return MATIO_E_UNKNOWN_ERROR;
                }
            }
        } else {
            /* Structure should be a scalar */
            matvar->rank = 2;
            matvar->dims = (size_t*)malloc(2*sizeof(*matvar->dims));
            if ( NULL != matvar->dims ) {
                matvar->dims[0] = 1;
                matvar->dims[1] = 1;
                nelems = 1;
            } else {
                H5Tclose(field_type_id);
                H5Dclose(field_id);
                Mat_Critical("Error allocating memory for matvar->dims");
                return MATIO_E_UNKNOWN_ERROR;
            }
        }
        H5Tclose(field_type_id);
        H5Dclose(field_id);
    } else {
        /* Structure should be a scalar */
        matvar->rank = 2;
        matvar->dims = (size_t*)malloc(2*sizeof(*matvar->dims));
        if ( NULL != matvar->dims ) {
            matvar->dims[0] = 1;
            matvar->dims[1] = 1;
            nelems = 1;
        } else {
            Mat_Critical("Error allocating memory for matvar->dims");
            return MATIO_E_OUT_OF_MEMORY;
        }
    }

    if ( nelems < 1 || nfields < 1 )
        return err;

    matvar->data_size = sizeof(*fields);
    {
        size_t nelems_x_nfields;
        err = Mul(&nelems_x_nfields, nelems, nfields);
        err |= Mul(&matvar->nbytes, nelems_x_nfields, matvar->data_size);
        if ( err ) {
            Mat_Critical("Integer multiplication overflow");
            matvar->nbytes = 0;
            return err;
        }
    }
    fields = (matvar_t**)calloc(matvar->nbytes, 1);
    matvar->data = fields;
    if ( NULL != fields ) {
        hsize_t k;
        for ( k = 0; k < nfields; k++ ) {
            H5O_INFO_T object_info;
            fields[k] = NULL;
            object_info.type = H5O_TYPE_UNKNOWN;
            H5OGET_INFO_BY_NAME(dset_id, matvar->internal->fieldnames[k], &object_info, H5P_DEFAULT);
            if ( object_info.type == H5O_TYPE_DATASET ) {
                field_id = H5Dopen(dset_id,matvar->internal->fieldnames[k], H5P_DEFAULT);
                if ( !fields_are_variables ) {
                    hobj_ref_t *ref_ids = (hobj_ref_t*)calloc((size_t)nelems, sizeof(*ref_ids));
                    if ( ref_ids != NULL ) {
                        hsize_t l;
                        herr_t herr = H5Dread(field_id,H5T_STD_REF_OBJ,H5S_ALL,H5S_ALL,
                            H5P_DEFAULT,ref_ids);
                        if ( herr < 0 ) {
                            err = MATIO_E_GENERIC_READ_ERROR;
                        } else {
                            for ( l = 0; l < nelems; l++ ) {
                                hid_t ref_id;
                                fields[l*nfields+k] = Mat_VarCalloc();
                                fields[l*nfields+k]->name =
                                    strdup(matvar->internal->fieldnames[k]);
                                fields[l*nfields+k]->internal->hdf5_ref=ref_ids[l];
                                /* Closing of ref_id is done in Mat_H5ReadNextReferenceInfo */
                                ref_id = H5RDEREFERENCE(field_id,H5R_OBJECT,ref_ids+l);
                                if ( ref_id < 0 ) {
                                    err = MATIO_E_GENERIC_READ_ERROR;
                                } else {
                                    fields[l*nfields+k]->internal->id = ref_id;
                                    err = Mat_H5ReadNextReferenceInfo(ref_id,fields[l*nfields+k],mat);
                                }
                                if ( err ) {
                                    break;
                                }
                            }
                        }
                        free(ref_ids);
                    } else {
                        err = MATIO_E_OUT_OF_MEMORY;
                    }
                } else {
                    fields[k] = Mat_VarCalloc();
                    fields[k]->name = strdup(matvar->internal->fieldnames[k]);
                    err = Mat_H5ReadDatasetInfo(mat,fields[k],field_id);
                }
                H5Dclose(field_id);
            } else if ( object_info.type == H5O_TYPE_GROUP ) {
                field_id = H5Gopen(dset_id,matvar->internal->fieldnames[k],
                                   H5P_DEFAULT);
                if ( -1 < field_id ) {
                    fields[k] = Mat_VarCalloc();
                    fields[k]->name = strdup(matvar->internal->fieldnames[k]);
                    err = Mat_H5ReadGroupInfo(mat,fields[k],field_id);
                    H5Gclose(field_id);
                }
            }
            if ( err ) {
                break;
            }
        }
    } else {
        err = MATIO_E_OUT_OF_MEMORY;
    }

    return err;