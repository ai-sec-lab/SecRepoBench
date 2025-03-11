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
        // Extract a null-terminated string from the input data into a buffer, checking for maximum length.
        // Allocate memory using the provided allocation function to store the string, including its null terminator.
        // Copy the string into the newly allocated memory and store the pointer in the output structure.
        // Mark the string in the output structure as dynamically allocated.
        // <MASK>
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