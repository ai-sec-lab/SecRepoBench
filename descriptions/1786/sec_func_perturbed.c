static int NITFLoadVQTables( NITFImage *psImage, int bTryGuessingOffset )

{
    int     i;
    GUInt32 nVQTableOffset=0 /*, nVQSize=0 */;
    GByte abyTestChunk[1000];
    const GByte abySignature[6] = { 0x00, 0x00, 0x00, 0x06, 0x00, 0x0E };

/* -------------------------------------------------------------------- */
/*      Do we already have the VQ tables?                               */
/* -------------------------------------------------------------------- */
    if( psImage->apanVQLUT[0] != NULL )
        return TRUE;

/* -------------------------------------------------------------------- */
/*      Do we have the location information?                            */
/* -------------------------------------------------------------------- */
    for( i = 0; i < psImage->nLocCount; i++ )
    {
        if( psImage->pasLocations[i].nLocId == LID_CompressionLookupSubsection)
        {
            nVQTableOffset = psImage->pasLocations[i].nLocOffset;
            /* nVQSize = psImage->pasLocations[i].nLocSize; */
        }
    }

    if( nVQTableOffset == 0 )
        return FALSE;

/* -------------------------------------------------------------------- */
/*      Does it look like we have the tables properly identified?       */
/* -------------------------------------------------------------------- */
    if( VSIFSeekL( psImage->psFile->fp, nVQTableOffset, SEEK_SET ) != 0 ||
        VSIFReadL( abyTestChunk, sizeof(abyTestChunk), 1, psImage->psFile->fp ) != 1 )
    {
        return FALSE;
    }

    if( memcmp(abyTestChunk,abySignature,sizeof(abySignature)) != 0 )
    {
        int bFoundSignature = FALSE;
        if (!bTryGuessingOffset)
            return FALSE;

        for( i = 0; (size_t)i < sizeof(abyTestChunk) - sizeof(abySignature); i++ )
        {
            if( memcmp(abyTestChunk+i,abySignature,sizeof(abySignature)) == 0 )
            {
                bFoundSignature = TRUE;
                nVQTableOffset += i;
                CPLDebug( "NITF",
                          "VQ CompressionLookupSubsection offsets off by %d bytes, adjusting accordingly.",
                          i );
                break;
            }
        }
        if (!bFoundSignature)
            return FALSE;
    }

/* -------------------------------------------------------------------- */
/*      Load the tables.                                                */
/* -------------------------------------------------------------------- */
    for( i = 0; i < 4; i++ )
    {
        GUInt32 nVQVector;
        int bOK;

        psImage->apanVQLUT[i] = (GUInt32 *) CPLCalloc(4096,sizeof(GUInt32));

        bOK = VSIFSeekL( psImage->psFile->fp, nVQTableOffset + 6 + i*14 + 10, SEEK_SET ) == 0;
        bOK &= VSIFReadL( &nVQVector, 1, 4, psImage->psFile->fp ) == 4;
        nVQVector = CPL_MSBWORD32( nVQVector );

        bOK &= VSIFSeekL( psImage->psFile->fp, nVQTableOffset + nVQVector, SEEK_SET ) == 0;
        bOK &= VSIFReadL( psImage->apanVQLUT[i], 4, 4096, psImage->psFile->fp ) == 4096;
        if( !bOK )
        {
            for( i = 0; i < 4; i++ )
            {
                CPLFree( psImage->apanVQLUT[i] );
                psImage->apanVQLUT[i] = NULL;
            }
            return FALSE;
        }
    }

    return TRUE;
}