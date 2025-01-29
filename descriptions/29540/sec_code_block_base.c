{
        int err;
        size_t nelems_x_nfields;
        err = Mul(&nelems_x_nfields, nelems, nfields);
        err |= Mul(&matvar->nbytes, nelems_x_nfields, matvar->data_size);
        if ( err ) {
            Mat_Critical("Integer multiplication overflow");
            matvar->nbytes = 0;
            return;
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
                field_id = H5Dopen(dset_id,matvar->internal->fieldnames[k],
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
                            strdup(matvar->internal->fieldnames[k]);
                        fields[l*nfields+k]->internal->hdf5_ref=ref_ids[l];
                        /* Closing of ref_id is done in Mat_H5ReadNextReferenceInfo */
                        ref_id = H5RDEREFERENCE(field_id,H5R_OBJECT,ref_ids+l);
                        fields[l*nfields+k]->internal->id = ref_id;
                        Mat_H5ReadNextReferenceInfo(ref_id,fields[l*nfields+k],mat);
                    }
                    free(ref_ids);
                } else {
                    fields[k] = Mat_VarCalloc();
                    fields[k]->name = strdup(matvar->internal->fieldnames[k]);
                    Mat_H5ReadDatasetInfo(mat,fields[k],field_id);
                }
                H5Dclose(field_id);
            } else if ( object_info.type == H5O_TYPE_GROUP ) {
                field_id = H5Gopen(dset_id,matvar->internal->fieldnames[k],
                                   H5P_DEFAULT);
                if ( -1 < field_id ) {
                    fields[k] = Mat_VarCalloc();
                    fields[k]->name = strdup(matvar->internal->fieldnames[k]);
                    Mat_H5ReadGroupInfo(mat,fields[k],field_id);
                    H5Gclose(field_id);
                }
            }
        }
    }