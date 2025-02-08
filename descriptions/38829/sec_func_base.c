static int
Mat_H5ReadNextReferenceData(hid_t ref_id, matvar_t *matvar, mat_t *mat)
{
    int err = MATIO_E_NO_ERROR;
    size_t nelems = 1;

    if ( ref_id < 0 || matvar == NULL )
        return err;

    /* If the datatype with references is a cell, we've already read info into
     * the variable data, so just loop over each cell element and call
     * Mat_H5ReadNextReferenceData on it.
     */
    if ( MAT_C_CELL == matvar->class_type ) {
        size_t i;
        matvar_t **cells;

        if ( NULL == matvar->data ) {
            return err;
        }
        err = Mat_MulDims(matvar, &nelems);
        if ( err ) {
            return err;
        }
        cells = (matvar_t **)matvar->data;
        for ( i = 0; i < nelems; i++ ) {
            if ( NULL != cells[i] ) {
                err = Mat_H5ReadNextReferenceData(cells[i]->internal->id, cells[i], mat);
            }
            if ( err ) {
                break;
            }
        }
        return err;
    }

    switch ( H5Iget_type(ref_id) ) {
        case H5I_DATASET: {
            hid_t data_type_id, dset_id;
            if ( MAT_C_CHAR == matvar->class_type ) {
                matvar->data_size = (int)Mat_SizeOf(matvar->data_type);
                data_type_id = DataType2H5T(matvar->data_type);
            } else if ( MAT_C_STRUCT == matvar->class_type ) {
                /* Empty structure array */
                break;
            } else {
                matvar->data_size = (int)Mat_SizeOfClass(matvar->class_type);
                data_type_id = ClassType2H5T(matvar->class_type);
            }

            err = Mat_MulDims(matvar, &nelems);
            err |= Mul(&matvar->nbytes, nelems, matvar->data_size);
            if ( err || matvar->nbytes < 1 ) {
                H5Dclose(ref_id);
                break;
            }

            dset_id = ref_id;

            if ( !matvar->isComplex ) {
                matvar->data = malloc(matvar->nbytes);
            } else {
                matvar->data = ComplexMalloc(matvar->nbytes);
            }
            if ( NULL != matvar->data ) {
                err = Mat_H5ReadData(dset_id, data_type_id, H5S_ALL, H5S_ALL, matvar->isComplex,
                                     matvar->data);
            }
            H5Dclose(dset_id);
            break;
        }
        case H5I_GROUP: {
            if ( MAT_C_SPARSE == matvar->class_type ) {
                err = Mat_VarRead73(mat, matvar);
            } else {
                matvar_t **fields;
                size_t i;

                if ( !matvar->nbytes || !matvar->data_size || NULL == matvar->data )
                    break;
                nelems = matvar->nbytes / matvar->data_size;
                fields = (matvar_t **)matvar->data;
                for ( i = 0; i < nelems; i++ ) {
                    if ( NULL != fields[i] && 0 < fields[i]->internal->hdf5_ref &&
                         -1 < fields[i]->internal->id ) {
                        /* Dataset of references */
                        err = Mat_H5ReadNextReferenceData(fields[i]->internal->id, fields[i], mat);
                    } else {
                        err = Mat_VarRead73(mat, fields[i]);
                    }
                    if ( err ) {
                        break;
                    }
                }
            }
            break;
        }
        default:
            break;
    }

    return err;
}