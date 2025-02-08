static
bool ReadDenseNodes( GByte* pabyDataBuffer, GByte* pabyDataLimit,
                     OSMContext* psCtxt )
{
    GByte* pabyDataIDs = NULL;
    GByte* pabyDataIDsLimit = NULL;
    GByte* pabyDataLat = NULL;
    GByte* pabyDataLon = NULL;
    GByte* apabyData[DENSEINFO_IDX_VISIBLE] = {NULL, NULL, NULL, NULL, NULL, NULL};
    GByte* pabyDataKeyVal = NULL;
    // <MASK>

    /* printf("<ReadDenseNodes\n"); */

    return TRUE;

end_error:
    /* printf("<ReadDenseNodes\n"); */

    return FALSE;
}