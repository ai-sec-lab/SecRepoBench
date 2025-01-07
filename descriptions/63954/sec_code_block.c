
    TABLE* t = GetTable(it8);
    int nSamples    = t -> nSamples;
    int nPatches    = t -> nPatches;

    if (nSet < 0 || nSet >= nPatches || nField < 0 || nField >= nSamples)
        return NULL;

    if (!t->Data) return NULL;
    return t->Data [nSet * nSamples + nField];
