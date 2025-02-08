CPLErr DecompressCCITTRLETile( unsigned char *pabySrcData, int nSrcBytes,
                               unsigned char *pabyDstData, int nDstBytes,
                               int nBlockXSize,
                               CPL_UNUSED int nBlockYSize )
{
    Fax3DecodeState  sDecoderState;
    Fax3BaseState* sp = (Fax3BaseState *) &sDecoderState;
    unsigned char runs_buf[4000];
    long rowbytes, rowpixels;

    memset( &sDecoderState, 0, sizeof(sDecoderState) );

    sp->groupoptions = 0;
    sp->recvparams = 0;
    sp->subaddress = NULL;

    DecoderState(sp)->runs = NULL;
    DecoderState(sp)->fill = aig_TIFFFax3fillruns;

    if( sizeof(runs_buf) < (size_t)(nBlockXSize * 2 + 3) )
    {
        CPLError(CE_Failure, CPLE_AppDefined, "Run buffer too small");
        return CE_Failure;
    }

/* -------------------------------------------------------------------- */
/*                                                                      */
/* -------------------------------------------------------------------- */
    /*
     * Calculate the scanline/tile widths.
     */
    // <MASK>
}