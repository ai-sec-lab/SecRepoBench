int nSamples    = t -> nSamples;
    int nPatches    = t -> nPatches;

    if (setIndex < 0 || setIndex >= nPatches || nField < 0 || nField >= nSamples)
        return NULL;

    if (!t->Data) return NULL;