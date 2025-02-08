/* -------------------------------------------------------------------- */
/*      Do we have an NITF app tag?  If so, pull out the Q level.       */
/* -------------------------------------------------------------------- */
    if( memcmp(abyHeader+nOffset+6,"NITF\0",5) != 0 )
        return 0;

    return abyHeader[22+nOffset];