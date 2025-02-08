int NITFDataset::ScanJPEGQLevel( GUIntBig *pDataOffset, bool *pbError )

{
    if( VSIFSeekL( psFile->fp, *pDataOffset,
                   SEEK_SET ) != 0 )
    {
        CPLError( CE_Failure, CPLE_FileIO,
                  "Seek error to jpeg data stream." );
        *pbError = true;
        return 0;
    }

    GByte abyHeader[100];
    if( VSIFReadL( abyHeader, 1, sizeof(abyHeader), psFile->fp )
        < sizeof(abyHeader) )
    {
        CPLError( CE_Failure, CPLE_FileIO,
                  "Read error to jpeg data stream." );
        *pbError = true;
        return 0;
    }

/* -------------------------------------------------------------------- */
/*      Scan ahead for jpeg magic code.  In some files (eg. NSIF)       */
/*      there seems to be some extra junk before the image data stream. */
/* -------------------------------------------------------------------- */
    GUInt32 nOffset = 0;
    while( nOffset < sizeof(abyHeader) - 23
           && (abyHeader[nOffset+0] != 0xff
               || abyHeader[nOffset+1] != 0xd8
               || abyHeader[nOffset+2] != 0xff) )
        nOffset++;

    if( nOffset >= sizeof(abyHeader) - 23 )
    {
        *pbError = true;
        return 0;
    }

    *pbError = false;
    *pDataOffset += nOffset;

    if( nOffset > 0 )
        CPLDebug( "NITF",
                  "JPEG data stream at offset %d from start of data segment, "
                  "NSIF?",
                  nOffset );

/* -------------------------------------------------------------------- */
/*      Do we have an NITF app tag?  If so, pull out the Q level.       */
/* -------------------------------------------------------------------- */
    if( memcmp(abyHeader+nOffset+6,"NITF\0",5) != 0 )
        return 0;

    return abyHeader[22+nOffset];
}