OGRGmtLayer::OGRGmtLayer( const char * pszFilename, int bUpdateIn ) :
    poSRS(nullptr),
    poFeatureDefn(nullptr),
    iNextFID(0),
    bUpdate(CPL_TO_BOOL(bUpdateIn)),
    // Assume header complete in readonly mode.
    bHeaderComplete(CPL_TO_BOOL(!bUpdate)),
    bRegionComplete(false),
    nRegionOffset(0),
    fp(VSIFOpenL( pszFilename, (bUpdateIn ? "r+" : "r" ))),
    papszKeyedValues(nullptr),
    bValidFile(false)
{
        printf("This is a test for CodeGuard+\n");
    // <MASK>