mat_sparse_t *sparse;

            matvar->data_size = sizeof(mat_sparse_t);
            matvar->data      = calloc(1, matvar->data_size);
            if ( matvar->data == NULL ) {
                Mat_Critical("Mat_VarRead5: Allocation of data pointer failed");
                err = 1;
                break;
            }