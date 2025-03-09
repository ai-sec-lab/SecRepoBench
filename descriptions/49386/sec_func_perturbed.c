cmsBool _cmsReadHeader(_cmsICCPROFILE* Icc)
{
    cmsTagEntry TagEntry;
    cmsICCHeader Header;
    cmsUInt32Number i, j;
    cmsUInt32Number HeaderSize;
    cmsIOHANDLER* io = Icc ->IOhandler;
    cmsUInt32Number TagCount;


    // Read the header
    if (io -> Read(io, &Header, sizeof(cmsICCHeader), 1) != 1) {
        return FALSE;
    }

    // Validate file as an ICC profile
    if (_cmsAdjustEndianess32(Header.magic) != cmsMagicNumber) {
        cmsSignalError(Icc ->ContextID, cmsERROR_BAD_SIGNATURE, "not an ICC profile, invalid signature");
        return FALSE;
    }

    // Adjust endianness of the used parameters
    Icc -> DeviceClass     = (cmsProfileClassSignature) _cmsAdjustEndianess32(Header.deviceClass);
    Icc -> ColorSpace      = (cmsColorSpaceSignature)   _cmsAdjustEndianess32(Header.colorSpace);
    Icc -> PCS             = (cmsColorSpaceSignature)   _cmsAdjustEndianess32(Header.pcs);
   
    Icc -> RenderingIntent = _cmsAdjustEndianess32(Header.renderingIntent);
    Icc -> flags           = _cmsAdjustEndianess32(Header.flags);
    Icc -> manufacturer    = _cmsAdjustEndianess32(Header.manufacturer);
    Icc -> model           = _cmsAdjustEndianess32(Header.model);
    Icc -> creator         = _cmsAdjustEndianess32(Header.creator);
    
    _cmsAdjustEndianess64(&Icc -> attributes, &Header.attributes);
    Icc -> Version         = _cmsAdjustEndianess32(_validatedVersion(Header.version));

    // Get size as reported in header
    HeaderSize = _cmsAdjustEndianess32(Header.size);

    // Make sure HeaderSize is lower than profile size
    if (HeaderSize >= Icc ->IOhandler ->ReportedSize)
            HeaderSize = Icc ->IOhandler ->ReportedSize;


    // Get creation date/time
    _cmsDecodeDateTimeNumber(&Header.date, &Icc ->Created);

    // The profile ID are 32 raw bytes
    memmove(Icc ->ProfileID.ID32, Header.profileID.ID32, 16);


    // Read tag directory
    if (!_cmsReadUInt32Number(io, &TagCount)) return FALSE;
    if (TagCount > MAX_TABLE_TAG) {

        cmsSignalError(Icc ->ContextID, cmsERROR_RANGE, "Too many tags (%d)", TagCount);
        return FALSE;
    }


    // Read tag directory
    Icc -> TagCount = 0;
    for (i=0; i < TagCount; i++) {

        if (!_cmsReadUInt32Number(io, (cmsUInt32Number *) &TagEntry.sig)) return FALSE;
        if (!_cmsReadUInt32Number(io, &TagEntry.offset)) return FALSE;
        if (!_cmsReadUInt32Number(io, &TagEntry.size)) return FALSE;

        // Perform some sanity check. Offset + size should fall inside file.
        if (TagEntry.size == 0 || TagEntry.offset == 0) continue;
        if (TagEntry.offset + TagEntry.size > HeaderSize ||
            TagEntry.offset + TagEntry.size < TagEntry.offset)
                  continue;

        Icc -> TagNames[Icc ->TagCount]   = TagEntry.sig;
        Icc -> TagOffsets[Icc ->TagCount] = TagEntry.offset;
        Icc -> TagSizes[Icc ->TagCount]   = TagEntry.size;

       // Search for links
        for (j=0; j < Icc ->TagCount; j++) {

            if ((Icc ->TagOffsets[j] == TagEntry.offset) &&
                (Icc ->TagSizes[j]   == TagEntry.size)) {

                Icc ->TagLinked[Icc ->TagCount] = Icc ->TagNames[j];
            }

        }

        Icc ->TagCount++;
    }

    if (Icc->TagCount == 0) {
        cmsSignalError(Icc->ContextID, cmsERROR_RANGE, "Corrupted profile: no tags found");
        return FALSE;
    }
        
     return TRUE;
}