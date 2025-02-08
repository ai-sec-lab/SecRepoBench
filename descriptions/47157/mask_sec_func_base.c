static int
pdfi_t1_decode_pfb(pdf_context *ctx, byte *inbuf, int inlen, byte **outbuf, int *outlen)
{
    stream *strm;
    int c, code = 0;
    int decodelen = 0;
    byte *d, *decodebuf = NULL;

    *outbuf = NULL;
    *outlen = 0;

    // <MASK>
}