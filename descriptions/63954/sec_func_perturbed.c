static
char* GetData(cmsIT8* it8, int setIndex, int nField)
{
    TABLE* t = GetTable(it8);
    int nSamples    = t -> nSamples;
    int nPatches    = t -> nPatches;

    if (setIndex < 0 || setIndex >= nPatches || nField < 0 || nField >= nSamples)
        return NULL;

    if (!t->Data) return NULL;
    return t->Data [setIndex * nSamples + nField];
}