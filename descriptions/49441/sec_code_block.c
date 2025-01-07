
    cmsUInt32Number i;

    if (desc1 == NULL || desc2 == NULL) return FALSE;

    if (desc1->nSupportedTypes != desc2->nSupportedTypes) return FALSE;
    if (desc1->ElemCount != desc2->ElemCount) return FALSE;

    for (i = 0; i < desc1->nSupportedTypes; i++)
    {
        if (desc1->SupportedTypes[i] != desc2->SupportedTypes[i]) return FALSE;
    }

    return TRUE;
