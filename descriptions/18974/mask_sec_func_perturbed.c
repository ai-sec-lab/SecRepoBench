static size_t
ReadNextStructField( mat_t *mat, matvar_t *matvar )
{
    mat_uint32_t fieldname_size;
    int err;
    size_t bytesread = 0, nfields, fieldindex;
    matvar_t **fields = NULL;
    size_t nelems = 1, nelems_x_nfields;

    err = SafeMulDims(matvar, &nelems);
    if ( err ) {
        Mat_Critical("Integer multiplication overflow");
        return bytesread;
    }
    if ( matvar->compression == MAT_COMPRESSION_ZLIB ) // <MASK>

    return bytesread;
}