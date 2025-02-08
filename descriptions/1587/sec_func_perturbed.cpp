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
    const int nTotalHdrSizeUncompressed = nSectionSize * numSections;

    const int nVertexSize =
                isCompressed ? 2 * sizeof(GUInt16) : 2 * sizeof(GUInt32);
    numVerticesTotal = 0;

    for( int i = 0;  i < numSections; i++ )
    {
        /*-------------------------------------------------------------
         * Read the coord. section header blocks
         *------------------------------------------------------------*/
#ifdef TABDUMP
        int nHdrAddress = GetCurAddress();
#endif
        if (nVersion >= 450)
            pasHdrs[i].numVertices = ReadInt32();
        else
            pasHdrs[i].numVertices = ReadInt16();

        if( pasHdrs[i].numVertices < 0 ||
            pasHdrs[i].numVertices > INT_MAX / nVertexSize )
        {
            CPLError(CE_Failure, CPLE_AssertionFailed,
                     "Invalid number of vertices for section %d", i);
            return -1;
        }
        if (nVersion >= 800)
            pasHdrs[i].numHoles = ReadInt32();
        else
            pasHdrs[i].numHoles = ReadInt16();
        if( pasHdrs[i].numHoles < 0 )
        {
            CPLError(CE_Failure, CPLE_AssertionFailed,
                     "Invalid number of holes for section %d", i);
            return -1;
        }
        ReadIntCoord(isCompressed, pasHdrs[i].nXMin, pasHdrs[i].nYMin);
        ReadIntCoord(isCompressed, pasHdrs[i].nXMax, pasHdrs[i].nYMax);
        pasHdrs[i].nDataOffset = ReadInt32();
        if( pasHdrs[i].nDataOffset < nTotalHdrSizeUncompressed )
        {
            CPLError(CE_Failure, CPLE_AssertionFailed,
                     "Invalid data offset for section %d", i);
            return -1;
        }

        if (CPLGetLastErrorType() != 0)
            return -1;

        if( numVerticesTotal > INT_MAX / nVertexSize - pasHdrs[i].numVertices )
        {
            CPLError(CE_Failure, CPLE_AssertionFailed,
                     "Invalid number of vertices for section %d", i);
            return -1;
        }
        numVerticesTotal += pasHdrs[i].numVertices;

        pasHdrs[i].nVertexOffset = (pasHdrs[i].nDataOffset -
                                    nTotalHdrSizeUncompressed ) / 8;
#ifdef TABDUMP
        printf("READING pasHdrs[%d] @ %d = \n"/*ok*/
               "              { numVertices = %d, numHoles = %d, \n"
               "                nXMin=%d, nYMin=%d, nXMax=%d, nYMax=%d,\n"
               "                nDataOffset=%d, nVertexOffset=%d }\n",
               i, nHdrAddress, pasHdrs[i].numVertices, pasHdrs[i].numHoles,
               pasHdrs[i].nXMin, pasHdrs[i].nYMin, pasHdrs[i].nXMax,
               pasHdrs[i].nYMax, pasHdrs[i].nDataOffset,
               pasHdrs[i].nVertexOffset);
        printf("                dX = %d, dY = %d  (center = %d , %d)\n",/*ok*/
               pasHdrs[i].nXMax - pasHdrs[i].nXMin,
               pasHdrs[i].nYMax - pasHdrs[i].nYMin,
               m_nComprOrgX, m_nComprOrgY);
#endif
    }

    for( int i = 0; i < numSections; i++ )
    {
        /*-------------------------------------------------------------
         * Make sure all coordinates are grouped together
         * (Well... at least check that all the vertex indices are enclosed
         * inside the [0..numVerticesTotal] range.)
         *------------------------------------------------------------*/
        if ( pasHdrs[i].nVertexOffset < 0 ||
             pasHdrs[i].nVertexOffset > INT_MAX - pasHdrs[i].numVertices ||
             (pasHdrs[i].nVertexOffset +
                           pasHdrs[i].numVertices ) > numVerticesTotal)
        {
            CPLError(CE_Failure, CPLE_AssertionFailed,
                     "Unsupported case or corrupt file: MULTIPLINE/REGION "
                     "object vertices do not appear to be grouped together.");
            return -1;
        }
    }

    return 0;
}