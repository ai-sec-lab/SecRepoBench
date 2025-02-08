void OGRSimpleCurve::setNumPoints( int nNewPointCount, int bZeroizeNewContent )

{
    CPLAssert( nNewPointCount >= 0 );

    if( nNewPointCount == 0 )
    {
        CPLFree( paoPoints );
        paoPoints = nullptr;

        CPLFree( padfZ );
        padfZ = nullptr;

        CPLFree( padfM );
        padfM = nullptr;

        nPointCount = 0;
        return;
    }

    if( nNewPointCount > nPointCount )
    {
        // Overflow of sizeof(OGRRawPoint) * nNewPointCount can only occur on
        // 32 bit, but we don't really want to allocate 2 billion points even on
        // 64 bit...
        // <MASK>
    }

    nPointCount = nNewPointCount;
}