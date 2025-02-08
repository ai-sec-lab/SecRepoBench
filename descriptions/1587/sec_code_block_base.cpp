const int nTotalHdrSizeUncompressed = nSectionSize * numSections;

    const int nVertexSize =
                bCompressed ? 2 * sizeof(GUInt16) : 2 * sizeof(GUInt32);
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
        ReadIntCoord(bCompressed, pasHdrs[i].nXMin, pasHdrs[i].nYMin);
        ReadIntCoord(bCompressed, pasHdrs[i].nXMax, pasHdrs[i].nYMax);
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