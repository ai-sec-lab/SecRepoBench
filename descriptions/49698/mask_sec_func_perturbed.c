exr_result_t
internal_huf_decompress (
    exr_decode_pipeline_t* decode,
    const uint8_t*         compressed,
    uint64_t               nCompressed,
    uint16_t*              raw,
    uint64_t               nRawSize,
    void*                  spare,
    uint64_t               sparebytes)
{
    uint32_t                            im, iM, nBits;
    uint64_t                            nBytes;
    const uint8_t*                      ptr;
    exr_result_t                        rv;
    const struct _internal_exr_context* pctxt = NULL;
    // <MASK>
    return rv;
}