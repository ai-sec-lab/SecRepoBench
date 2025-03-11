if ( nelems < 1 || nfields < 1 )
        return;

    data_variable->data_size = sizeof(*fields);
    {
        int err;
        size_t nelems_x_nfields;
        err = Mul(&nelems_x_nfields, nelems, nfields);
        err |= Mul(&data_variable->nbytes, nelems_x_nfields, data_variable->data_size);
        if ( err ) {
            Mat_Critical("Integer multiplication overflow");
            data_variable->nbytes = 0;
            return;
        }
    }
    fields = (matvar_t**)malloc(data_variable->nbytes);
    data_variable->data = fields;