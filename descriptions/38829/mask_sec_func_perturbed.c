static int
Mat_H5ReadNextReferenceData(hid_t ref_id, matvar_t *matvar, mat_t *mat)
{
    int errorCode = MATIO_E_NO_ERROR;
    size_t nelems = 1;

    if ( ref_id < 0 || matvar == NULL )
        return errorCode;

    /* If the datatype with references is a cell, we've already read info into
     * the variable data, so just loop over each cell element and call
     * Mat_H5ReadNextReferenceData on it.
     */
    if ( MAT_C_CELL == matvar->class_type ) {
        size_t i;
        matvar_t **cells;

        if ( NULL == matvar->data ) {
            return errorCode;
        }
        errorCode = Mat_MulDims(matvar, &nelems);
        if ( errorCode ) {
            return errorCode;
        }
        cells = (matvar_t **)matvar->data;
        for ( i = 0; i < nelems; i++ ) {
            if ( NULL != cells[i] ) {
                errorCode = Mat_H5ReadNextReferenceData(cells[i]->internal->id, cells[i], mat);
            }
            if ( errorCode ) {
                break;
            }
        }
        return errorCode;
    }

    switch ( H5Iget_type(ref_id) ) {
        case H5I_DATASET: {
            hid_t data_type_id, dset_id;
            if ( MAT_C_CHAR == matvar->class_type ) // <MASK>

            errorCode = Mat_MulDims(matvar, &nelems);
            errorCode |= Mul(&matvar->nbytes, nelems, matvar->data_size);
            if ( errorCode || matvar->nbytes < 1 ) {
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
                errorCode = Mat_H5ReadData(dset_id, data_type_id, H5S_ALL, H5S_ALL, matvar->isComplex,
                                     matvar->data);
            }
            H5Dclose(dset_id);
            break;
        }
        case H5I_GROUP: {
            if ( MAT_C_SPARSE == matvar->class_type ) {
                errorCode = Mat_VarRead73(mat, matvar);
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
                        errorCode = Mat_H5ReadNextReferenceData(fields[i]->internal->id, fields[i], mat);
                    } else {
                        errorCode = Mat_VarRead73(mat, fields[i]);
                    }
                    if ( errorCode ) {
                        break;
                    }
                }
            }
            break;
        }
        default:
            break;
    }

    return errorCode;
}