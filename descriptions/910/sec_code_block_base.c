if (TagSize < 8) goto Error;

    // Seek to its location
    if (!io -> Seek(io, Offset))
        goto Error;

    // Search for support on this tag
    TagDescriptor = _cmsGetTagDescriptor(Icc-> ContextID, sig);
    if (TagDescriptor == NULL) {

        char String[5];

        _cmsTagSignature2String(String, sig);

        // An unknown element was found.
        cmsSignalError(Icc ->ContextID, cmsERROR_UNKNOWN_EXTENSION, "Unknown tag type '%s' found.", String);
        goto Error;     // Unsupported.
    }

    // if supported, get type and check if in list
    BaseType = _cmsReadTypeBase(io);
    if (BaseType == 0) goto Error;

    if (!IsTypeSupported(TagDescriptor, BaseType)) goto Error;
   
    TagSize  -= 8;