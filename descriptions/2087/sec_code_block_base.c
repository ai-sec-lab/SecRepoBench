rowbytes = (nBlockXSize + 7) / 8;
    rowpixels = nBlockXSize;

    sp->rowbytes = (GUInt32) rowbytes;
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