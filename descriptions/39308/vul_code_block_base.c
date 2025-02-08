size_t nelems = 1;
    enum matio_types packed_type = MAT_T_UNKNOWN;
    long fpos;
    mat_uint32_t tag[2];
    size_t bytesread = 0;

    if ( matvar == NULL )
        return MATIO_E_BAD_ARGUMENT;
    else if ( matvar->rank == 0 ) /* An empty data set */
        return MATIO_E_NO_ERROR;