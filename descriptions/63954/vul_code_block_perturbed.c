int nSamples    = t -> nSamples;
    int nPatches    = t -> nPatches;

    if (setIndex >= nPatches || nField >= nSamples)
        return NULL;

    if (!t->Data) return NULL;