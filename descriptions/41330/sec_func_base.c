static size_t
ReadSparse(mat_t *mat, matvar_t *matvar, mat_uint32_t *n, mat_uint32_t **v)
{
    int data_in_tag = 0;
    enum matio_types packed_type;
    mat_uint32_t tag[2] = {0, 0};
    size_t bytesread = 0;
    mat_uint32_t N = 0;

    if ( matvar->compression == MAT_COMPRESSION_ZLIB ) {
#if HAVE_ZLIB
        matvar->internal->z->avail_in = 0;
        if ( 0 != Inflate(mat, matvar->internal->z, tag, 4, &bytesread) ) {
            return bytesread;
        }
        if ( mat->byteswap )
            (void)Mat_uint32Swap(tag);
        packed_type = TYPE_FROM_TAG(tag[0]);
        if ( tag[0] & 0xffff0000 ) { /* Data is in the tag */
            data_in_tag = 1;
            N = (tag[0] & 0xffff0000) >> 16;
        } else {
            data_in_tag = 0;
            (void)ReadCompressedUInt32Data(mat, matvar->internal->z, &N, MAT_T_UINT32, 1);
        }
#endif
    } else {
        if ( 0 != Read(tag, 4, 1, (FILE *)mat->fp, &bytesread) ) {
            return bytesread;
        }
        if ( mat->byteswap )
            (void)Mat_uint32Swap(tag);
        packed_type = TYPE_FROM_TAG(tag[0]);
        if ( tag[0] & 0xffff0000 ) { /* Data is in the tag */
            data_in_tag = 1;
            N = (tag[0] & 0xffff0000) >> 16;
        } else {
            data_in_tag = 0;
            if ( 0 != Read(&N, 4, 1, (FILE *)mat->fp, &bytesread) ) {
                return bytesread;
            }
            if ( mat->byteswap )
                (void)Mat_uint32Swap(&N);
        }
    }
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