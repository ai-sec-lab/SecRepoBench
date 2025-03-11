static exr_result_t
Classifier_read (
    void* (*alloc_fn) (size_t),
    Classifier*     out,
    const uint8_t** ptr,
    size_t*         length)
{
    const uint8_t* curin = *ptr;
    size_t         len   = 0;
    uint8_t        value;
    uint8_t        type;

    if (*length <= 3) return EXR_ERR_CORRUPT_CHUNK;

    {
        // maximum length of string plus one byte for terminating NULL
        char  suffix[128 + 1];
        char* mem;
        memset (suffix, 0, 128 + 1);
        for (; len < 128 + 1; ++len)
        {
            if (len > (*length - 3)) return EXR_ERR_CORRUPT_CHUNK;
            if (curin[len] == '\0') break;
            suffix[len] = (char) curin[len];
        }
        if (len == 128 + 1) return EXR_ERR_CORRUPT_CHUNK;
        // account for extra byte for nil terminator
        len += 1;

        mem = alloc_fn (len);
        if (!mem) return EXR_ERR_OUT_OF_MEMORY;

        memcpy (mem, suffix, len);
        out->_suffix       = mem;
        out->_stringStatic = DWA_CLASSIFIER_FALSE;
    }

    if (*length < len + 2 * sizeof (uint8_t)) return EXR_ERR_CORRUPT_CHUNK;

    curin += len;

    value = curin[0];
    type  = curin[1];

    curin += 2;

    *ptr = curin;
    *length -= len + 2 * sizeof (uint8_t);

    out->_cscIdx = (int) (value >> 4) - 1;
    if (out->_cscIdx < -1 || out->_cscIdx >= 3) return EXR_ERR_CORRUPT_CHUNK;

    out->_scheme = (CompressorScheme) ((value >> 2) & 3);
    if (out->_scheme >= NUM_COMPRESSOR_SCHEMES) return EXR_ERR_CORRUPT_CHUNK;

    out->_caseInsensitive =
        (value & 1 ? DWA_CLASSIFIER_TRUE : DWA_CLASSIFIER_FALSE);

    if (type >= EXR_PIXEL_LAST_TYPE) return EXR_ERR_CORRUPT_CHUNK;

    out->_type = (exr_pixel_type_t) type;
    return EXR_ERR_SUCCESS;
}