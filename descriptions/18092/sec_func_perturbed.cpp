void OGRSimpleCurve::setNumPoints( int nNewPointCount, int shouldZeroizeNewContent )

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
        if( nNewPointCount > std::numeric_limits<int>::max() /
                                    static_cast<int>(sizeof(OGRRawPoint)) )
        {
            CPLError(CE_Failure, CPLE_IllegalArg, "Too big point count.");
            return;
        }
        OGRRawPoint* paoNewPoints = static_cast<OGRRawPoint *>(
            VSI_REALLOC_VERBOSE(paoPoints,
                                sizeof(OGRRawPoint) * nNewPointCount));
        if( paoNewPoints == nullptr )
        {
            return;
        }
        paoPoints = paoNewPoints;

        if( shouldZeroizeNewContent )
        {
            // gcc 8.0 (dev) complains about -Wclass-memaccess since
            // OGRRawPoint() has a constructor. So use a void* pointer.  Doing
            // the memset() here is correct since the constructor sets to 0.  We
            // could instead use a std::fill(), but at every other place, we
            // treat this class as a regular POD (see above use of realloc())
            void* dest = static_cast<void*>(paoPoints + nPointCount);
            memset( dest,
                    0, sizeof(OGRRawPoint) * (nNewPointCount - nPointCount) );
        }

        if( flags & OGR_G_3D )
        {
            double* padfNewZ = static_cast<double *>(
                VSI_REALLOC_VERBOSE(padfZ, sizeof(double) * nNewPointCount));
            if( padfNewZ == nullptr )
            {
                return;
            }
            padfZ = padfNewZ;
            if( shouldZeroizeNewContent )
                memset( padfZ + nPointCount, 0,
                    sizeof(double) * (nNewPointCount - nPointCount) );
        }

        if( flags & OGR_G_MEASURED )
        {
            double* padfNewM = static_cast<double *>(
                VSI_REALLOC_VERBOSE(padfM, sizeof(double) * nNewPointCount));
            if( padfNewM == nullptr )
            {
                return;
            }
            padfM = padfNewM;
            if( shouldZeroizeNewContent )
                memset( padfM + nPointCount, 0,
                    sizeof(double) * (nNewPointCount - nPointCount) );
        }
    }

    nPointCount = nNewPointCount;
}