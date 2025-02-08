static
char* GetData(cmsIT8* it8, int setIndex, int nField)
{
    TABLE* t = GetTable(it8);
    // <MASK>
    return t->Data [setIndex * nSamples + nField];
}