void* CMSEXPORT cmsReadTag(cmsHPROFILE hProfile, cmsTagSignature tagSignature)
{
    _cmsICCPROFILE* Icc = (_cmsICCPROFILE*) hProfile;
    cmsIOHANDLER* io = Icc ->IOhandler;
    cmsTagTypeHandler* TypeHandler;
    cmsTagTypeHandler LocalTypeHandler;
    cmsTagDescriptor*  TagDescriptor;
    cmsTagTypeSignature BaseType;
    cmsUInt32Number Offset, TagSize;
    cmsUInt32Number ElemCount;
    int n;

    if (!_cmsLockMutex(Icc->ContextID, Icc ->UsrMutex)) return NULL;

    n = _cmsSearchTag(Icc, tagSignature, TRUE);
    if (n < 0) goto Error;               // Not found, return NULL


    // If the element is already in memory, return the pointer
    if (Icc -> TagPtrs[n]) {

        if (Icc->TagTypeHandlers[n] == NULL) goto Error;

        // Sanity check
        BaseType = Icc->TagTypeHandlers[n]->Signature;
        if (BaseType == 0) goto Error;

        TagDescriptor = _cmsGetTagDescriptor(Icc->ContextID, tagSignature);
        if (TagDescriptor == NULL) goto Error;

        if (!IsTypeSupported(TagDescriptor, BaseType)) goto Error;

        if (Icc ->TagSaveAsRaw[n]) goto Error;  // We don't support read raw tags as cooked

        _cmsUnlockMutex(Icc->ContextID, Icc ->UsrMutex);
        return Icc -> TagPtrs[n];
    }

    // We need to read it. Get the offset and size to the file
    Offset    = Icc -> TagOffsets[n];
    TagSize   = Icc -> TagSizes[n];

    // Seek to the tag's location in the IO handler.
    // Check if the tag is supported.
    // If the type is unsupported, exit. 
    // Change the tag size to account for the tag's base type header.
    // <MASK>

    // Get type handler
    TypeHandler = _cmsGetTagTypeHandler(Icc ->ContextID, BaseType);
    if (TypeHandler == NULL) goto Error;
    LocalTypeHandler = *TypeHandler;


    // Read the tag
    Icc -> TagTypeHandlers[n] = TypeHandler;

    LocalTypeHandler.ContextID = Icc ->ContextID;
    LocalTypeHandler.ICCVersion = Icc ->Version;
    Icc -> TagPtrs[n] = LocalTypeHandler.ReadPtr(&LocalTypeHandler, io, &ElemCount, TagSize);

    // The tag type is supported, but something wrong happened and we cannot read the tag.
    // let know the user about this (although it is just a warning)
    if (Icc -> TagPtrs[n] == NULL) {

        char String[5];

        _cmsTagSignature2String(String, tagSignature);
        cmsSignalError(Icc ->ContextID, cmsERROR_CORRUPTION_DETECTED, "Corrupted tag '%s'", String);
        goto Error;
    }

    // This is a weird error that may be a symptom of something more serious, the number of
    // stored item is actually less than the number of required elements.
    if (ElemCount < TagDescriptor ->ElemCount) {

        char String[5];

        _cmsTagSignature2String(String, tagSignature);
        cmsSignalError(Icc ->ContextID, cmsERROR_CORRUPTION_DETECTED, "'%s' Inconsistent number of items: expected %d, got %d",
            String, TagDescriptor ->ElemCount, ElemCount);
    }


    // Return the data
    _cmsUnlockMutex(Icc->ContextID, Icc ->UsrMutex);
    return Icc -> TagPtrs[n];


    // Return error and unlock tha data
Error:
    _cmsUnlockMutex(Icc->ContextID, Icc ->UsrMutex);
    return NULL;
}