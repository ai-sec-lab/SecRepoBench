if (n < 0)
    {
        // Not found, return NULL
        _cmsUnlockMutex(Icc->ContextID, Icc->UsrMutex);
        return NULL;
    }