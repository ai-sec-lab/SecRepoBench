/* -------------------------------------------------------------------- */
/*      Do we have an NITF app tag?  If so, pull out the Q level.       */
/* -------------------------------------------------------------------- */
    if( !EQUAL((char *)abyHeader+nOffset+6,"NITF") )
        return 0;

    return abyHeader[22+nOffset];