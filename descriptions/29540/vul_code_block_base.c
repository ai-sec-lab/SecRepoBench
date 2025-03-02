if ( nelems < 1 || nfields < 1 )
        return;

    matvar->data_size = sizeof(*fields);
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
    fields = (matvar_t**)malloc(matvar->nbytes);
    matvar->data = fields;