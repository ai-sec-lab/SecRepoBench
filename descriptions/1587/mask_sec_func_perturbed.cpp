int     TABMAPCoordBlock::ReadCoordSecHdrs(GBool isCompressed,
                                           int nVersion,
                                           int numSections,
                                           TABMAPCoordSecHdr *pasHdrs,
                                           GInt32    &numVerticesTotal)
{
    CPLErrorReset();

    /*-------------------------------------------------------------
     * Note about header+vertices size vs compressed coordinates:
     * The uncompressed header sections are actually 16 bytes, but the
     * offset calculations are based on prior decompression of the
     * coordinates.  Our coordinate offset calculations have
     * to take this fact into account.
     * Also, V450 header section uses int32 instead of int16 for numVertices
     * and we add another 2 bytes to align with a 4 bytes boundary.
     * V800 header section uses int32 for numHoles but there is no need
     * for the 2 alignment bytes so the size is the same as V450
     *------------------------------------------------------------*/
    const int nSectionSize = (nVersion >= 450) ? 28 : 24;
    if( numSections > INT_MAX / nSectionSize )
    {
        CPLError(CE_Failure, CPLE_AssertionFailed,
                 "Invalid numSections");
        return -1;
    }
    // <MASK>
}