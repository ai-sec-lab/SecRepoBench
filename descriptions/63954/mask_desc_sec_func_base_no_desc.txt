static
char* GetData(cmsIT8* it8, int nSet, int nField)
{
    TABLE* t = GetTable(it8);
    // <MASK>
    return t->Data [nSet * nSamples + nField];
}