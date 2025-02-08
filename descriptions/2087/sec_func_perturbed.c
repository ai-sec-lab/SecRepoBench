CPLErr DecompressCCITTRLETile( unsigned char *pabySrcData, int nSrcBytes,
                               unsigned char *pabyDstData, int nDstBytes,
                               int nBlockXSize,
                               CPL_UNUSED int nBlockYSize )
{
    Fax3DecodeState  sDecoderState;
    Fax3BaseState* sp = (Fax3BaseState *) &sDecoderState;
    unsigned char runs_buf[4000];
    long scanlinebytes, rowpixels;

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
    scanlinebytes = (nBlockXSize + 7) / 8;
    rowpixels = nBlockXSize;

    sp->rowbytes = (GUInt32) scanlinebytes;
    sp->rowpixels = (GUInt32) rowpixels;
    sp->mode |= FAXMODE_BYTEALIGN;
    /*
     * Allocate any additional space required for decoding/encoding.
     */
    {
        Fax3DecodeState* dsp = DecoderState(sp);

        dsp->runs = (GUInt32*) runs_buf;
        dsp->curruns = dsp->runs;
        dsp->refruns = NULL;
    }

/* -------------------------------------------------------------------- */
/*                                                                      */
/* -------------------------------------------------------------------- */
    DecoderState(sp)->bit = 0;		/* force initial read */
    DecoderState(sp)->data = 0;
    DecoderState(sp)->EOLcnt = 0;	/* force initial scan for EOL */

    DecoderState(sp)->bitmap = aig_TIFFBitRevTable;

    if (DecoderState(sp)->refruns) {	/* init reference line to white */
        DecoderState(sp)->refruns[0] = (GUInt32) DecoderState(sp)->b.rowpixels;
        DecoderState(sp)->refruns[1] = 0;
    }

    if( Fax3DecodeRLE( sp, pabyDstData, nDstBytes,
                       pabySrcData, nSrcBytes ) == 1 )
        return CE_None;
    else
        return CE_Failure;
}