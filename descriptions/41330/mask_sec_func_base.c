static size_t
ReadSparse(mat_t *mat, matvar_t *matvar, mat_uint32_t *n, mat_uint32_t **v)
{
    // <MASK>
    if ( 0 == N )
        return bytesread;
    *n = N / 4;
    *v = (mat_uint32_t *)calloc(N, 1);
    if ( NULL != *v ) {
        int nBytes;
        if ( matvar->compression == MAT_COMPRESSION_NONE ) {
            nBytes = ReadUInt32Data(mat, *v, packed_type, *n);
            /*
                * If the data was in the tag we started on a 4-byte
                * boundary so add 4 to make it an 8-byte
                */
            nBytes *= Mat_SizeOf(packed_type);
            if ( data_in_tag )
                nBytes += 4;
            if ( (nBytes % 8) != 0 )
                (void)fseek((FILE *)mat->fp, 8 - (nBytes % 8), SEEK_CUR);
#if HAVE_ZLIB
        } else if ( matvar->compression == MAT_COMPRESSION_ZLIB ) {
            nBytes = ReadCompressedUInt32Data(mat, matvar->internal->z, *v, packed_type, *n);
            /*
                * If the data was in the tag we started on a 4-byte
                * boundary so add 4 to make it an 8-byte
                */
            if ( data_in_tag )
                nBytes += 4;
            if ( (nBytes % 8) != 0 )
                InflateSkip(mat, matvar->internal->z, 8 - (nBytes % 8), NULL);
#endif
        }
    } else {
        Mat_Critical("Couldn't allocate memory");
    }

    return bytesread;
}