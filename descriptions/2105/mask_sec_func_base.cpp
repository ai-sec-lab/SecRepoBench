int nwt_ParseHeader( NWT_GRID * pGrd, char *nwtHeader )
{
    /* double dfTmp; */

    if( nwtHeader[4] == '1' )
        pGrd->cFormat = 0x00;        // grd - surface type
    else if( nwtHeader[4] == '8' )
        pGrd->cFormat = 0x80;        //  grc classified type

    pGrd->stClassDict = NULL;

    memcpy( reinterpret_cast<void *>( &pGrd->fVersion ),
            reinterpret_cast<void *>( &nwtHeader[5] ),
            sizeof( pGrd->fVersion ) );
    CPL_LSBPTR32(&pGrd->fVersion);

    unsigned short usTmp;
    memcpy( reinterpret_cast<void *>( &usTmp ),
            reinterpret_cast<void *>( &nwtHeader[9] ),
            2 );
    CPL_LSBPTR16(&usTmp);
    pGrd->nXSide = static_cast<unsigned int>( usTmp );
    if( pGrd->nXSide == 0 )
    {
        memcpy( reinterpret_cast<void *>( &pGrd->nXSide ),
                reinterpret_cast<void *>( &nwtHeader[128] ),
                sizeof(pGrd->nXSide) );
        CPL_LSBPTR32(&pGrd->nXSide);
    }
    // <MASK>

    return TRUE;
}