static void
Mat_H5ReadGroupInfo(mat_t *mat,matvar_t *data_variable,hid_t dset_id)
{
    int      fields_are_variables = 1;
    hsize_t  nfields=0,nelems;
    hid_t    attr_id,field_id;
    matvar_t **fields;
    H5O_type_t obj_type;

    Mat_H5ReadVarInfo(data_variable,dset_id);

    /* Check if the variable is sparse */
    if ( H5Aexists_by_name(dset_id,".","MATLAB_sparse",H5P_DEFAULT) ) {
        hid_t sparse_dset_id;
        unsigned nrows = 0;

        attr_id = H5Aopen_by_name(dset_id,".","MATLAB_sparse",H5P_DEFAULT,H5P_DEFAULT);
        H5Aread(attr_id,H5T_NATIVE_UINT,&nrows);
        H5Aclose(attr_id);

        data_variable->class_type = MAT_C_SPARSE;

        sparse_dset_id = H5Dopen(dset_id,"jc",H5P_DEFAULT);
        data_variable->dims = Mat_H5ReadDims(sparse_dset_id, &nelems, &data_variable->rank);
        H5Dclose(sparse_dset_id);
        if ( NULL != data_variable->dims ) {
            if ( 1 == data_variable->rank ) {
                size_t* dims = (size_t*)realloc(data_variable->dims, 2*sizeof(*data_variable->dims));
                if ( NULL != dims ) {
                    data_variable->rank = 2;
                    data_variable->dims = dims;
                }
            }
            if ( 2 == data_variable->rank ) {
                data_variable->dims[1] = data_variable->dims[0] - 1;
                data_variable->dims[0] = nrows;
            }
        } else {
            return;
        }

        /* Test if dataset type is compound and if so if it's complex */
        if ( H5Lexists(dset_id,"data",H5P_DEFAULT) ) {
            hid_t type_id;
            sparse_dset_id = H5Dopen(dset_id,"data",H5P_DEFAULT);
            type_id = H5Dget_type(sparse_dset_id);
            if ( H5T_COMPOUND == H5Tget_class(type_id) ) {
                /* FIXME: Any more checks? */
                data_variable->isComplex = MAT_F_COMPLEX;
            }
            H5Tclose(type_id);
            H5Dclose(sparse_dset_id);
        }
        return;
    }

    /* Check if the structure defines its fields in MATLAB_fields */
    if ( H5Aexists_by_name(dset_id,".","MATLAB_fields",H5P_DEFAULT) ) {
        Mat_H5ReadFieldNames(data_variable, dset_id, &nfields);
    } else {
        H5G_info_t group_info;
        data_variable->internal->num_fields = 0;
        H5Gget_info(dset_id, &group_info);
        if ( group_info.nlinks > 0 ) {
            struct ReadGroupInfoIterData group_data = {0, NULL};
            herr_t herr;

            /* First iteration to retrieve number of relevant links */
            herr = H5Literate_by_name(dset_id, data_variable->internal->hdf5_name, H5_INDEX_NAME,
                H5_ITER_NATIVE, NULL, Mat_H5ReadGroupInfoIterate,
                (void *)&group_data, H5P_DEFAULT);
            if ( herr > 0 && group_data.nfields > 0 ) {
                data_variable->internal->fieldnames =
                    (char**)calloc((size_t)(group_data.nfields),sizeof(*data_variable->internal->fieldnames));
                group_data.nfields = 0;
                group_data.matvar = data_variable;
                if ( data_variable->internal->fieldnames != NULL ) {
                    /* Second iteration to fill fieldnames */
                    H5Literate_by_name(dset_id, data_variable->internal->hdf5_name, H5_INDEX_NAME,
                        H5_ITER_NATIVE, NULL, Mat_H5ReadGroupInfoIterate,
                        (void *)&group_data, H5P_DEFAULT);
                }
                data_variable->internal->num_fields = (unsigned)group_data.nfields;
                nfields = group_data.nfields;
            }
        }
    }

    if ( nfields > 0 ) {
        H5O_INFO_T object_info;
        object_info.type = H5O_TYPE_UNKNOWN;
        H5OGET_INFO_BY_NAME(dset_id, data_variable->internal->fieldnames[0], &object_info, H5P_DEFAULT);
        obj_type = object_info.type;
    } else {
        obj_type = H5O_TYPE_UNKNOWN;
    }
    if ( obj_type == H5O_TYPE_DATASET ) {
        hid_t field_type_id;
        field_id = H5Dopen(dset_id,data_variable->internal->fieldnames[0],H5P_DEFAULT);
        field_type_id = H5Dget_type(field_id);
        if ( H5T_REFERENCE == H5Tget_class(field_type_id) ) {
            /* Check if the field has the MATLAB_class attribute. If so, it
             * means the structure is a scalar. Otherwise, the dimensions of
             * the field dataset is the dimensions of the structure
             */
            if ( H5Aexists_by_name(field_id,".","MATLAB_class",H5P_DEFAULT) ) {
                data_variable->rank = 2;
                data_variable->dims = (size_t*)malloc(2*sizeof(*data_variable->dims));
                if ( NULL != data_variable->dims ) {
                    data_variable->dims[0] = 1;
                    data_variable->dims[1] = 1;
                    nelems = 1;
                } else {
                    H5Tclose(field_type_id);
                    H5Dclose(field_id);
                    Mat_Critical("Error allocating memory for matvar->dims");
                    return;
                }
            } else {
                data_variable->dims = Mat_H5ReadDims(field_id, &nelems, &data_variable->rank);
                if ( NULL != data_variable->dims ) {
                    fields_are_variables = 0;
                } else {
                    H5Tclose(field_type_id);
                    H5Dclose(field_id);
                    return;
                }
            }
        } else {
            /* Structure should be a scalar */
            data_variable->rank = 2;
            data_variable->dims = (size_t*)malloc(2*sizeof(*data_variable->dims));
            if ( NULL != data_variable->dims ) {
                data_variable->dims[0] = 1;
                data_variable->dims[1] = 1;
                nelems = 1;
            } else {
                H5Tclose(field_type_id);
                H5Dclose(field_id);
                Mat_Critical("Error allocating memory for matvar->dims");
                return;
            }
        }
        H5Tclose(field_type_id);
        H5Dclose(field_id);
    } else {
        /* Structure should be a scalar */
        data_variable->rank = 2;
        data_variable->dims = (size_t*)malloc(2*sizeof(*data_variable->dims));
        if ( NULL != data_variable->dims ) {
            data_variable->dims[0] = 1;
            data_variable->dims[1] = 1;
            nelems = 1;
        } else {
            Mat_Critical("Error allocating memory for matvar->dims");
            return;
        }
    }

    // <MASK>
    if ( NULL != fields ) {
        hsize_t k;
        for ( k = 0; k < nfields; k++ ) {
            H5O_INFO_T object_info;
            fields[k] = NULL;
            object_info.type = H5O_TYPE_UNKNOWN;
            H5OGET_INFO_BY_NAME(dset_id, data_variable->internal->fieldnames[k], &object_info, H5P_DEFAULT);
            if ( object_info.type == H5O_TYPE_DATASET ) {
                field_id = H5Dopen(dset_id,data_variable->internal->fieldnames[k],
                                   H5P_DEFAULT);
                if ( !fields_are_variables ) {
                    hsize_t l;
                    hobj_ref_t *ref_ids = (hobj_ref_t*)malloc((size_t)nelems*sizeof(*ref_ids));
                    H5Dread(field_id,H5T_STD_REF_OBJ,H5S_ALL,H5S_ALL,
                            H5P_DEFAULT,ref_ids);
                    for ( l = 0; l < nelems; l++ ) {
                        hid_t ref_id;
                        fields[l*nfields+k] = Mat_VarCalloc();
                        fields[l*nfields+k]->name =
                            strdup(data_variable->internal->fieldnames[k]);
                        fields[l*nfields+k]->internal->hdf5_ref=ref_ids[l];
                        /* Closing of ref_id is done in Mat_H5ReadNextReferenceInfo */
                        ref_id = H5RDEREFERENCE(field_id,H5R_OBJECT,ref_ids+l);
                        fields[l*nfields+k]->internal->id = ref_id;
                        Mat_H5ReadNextReferenceInfo(ref_id,fields[l*nfields+k],mat);
                    }
                    free(ref_ids);
                } else {
                    fields[k] = Mat_VarCalloc();
                    fields[k]->name = strdup(data_variable->internal->fieldnames[k]);
                    Mat_H5ReadDatasetInfo(mat,fields[k],field_id);
                }
                H5Dclose(field_id);
            } else if ( object_info.type == H5O_TYPE_GROUP ) {
                field_id = H5Gopen(dset_id,data_variable->internal->fieldnames[k],
                                   H5P_DEFAULT);
                if ( -1 < field_id ) {
                    fields[k] = Mat_VarCalloc();
                    fields[k]->name = strdup(data_variable->internal->fieldnames[k]);
                    Mat_H5ReadGroupInfo(mat,fields[k],field_id);
                    H5Gclose(field_id);
                }
            }
        }
    }
}