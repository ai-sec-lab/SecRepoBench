n = _cmsSearchTag(Icc, sig, TRUE);
    if (n < 0) goto Error;               // Not found, return NULL


    // If the element is already in memory, return the pointer
    if (Icc -> TagPtrs[n]) {

        if (Icc->TagTypeHandlers[n] == NULL) goto Error;

        // Sanity check
        BaseType = Icc->TagTypeHandlers[n]->Signature;
        if (BaseType == 0) goto Error;

        TagDescriptor = _cmsGetTagDescriptor(Icc->ContextID, sig);
        if (TagDescriptor == NULL) goto Error;

        if (!IsTypeSupported(TagDescriptor, BaseType)) goto Error;

        if (Icc ->TagSaveAsRaw[n]) goto Error;  // We don't support read raw tags as cooked

        _cmsUnlockMutex(Icc->ContextID, Icc ->UsrMutex);
        return Icc -> TagPtrs[n];
    }