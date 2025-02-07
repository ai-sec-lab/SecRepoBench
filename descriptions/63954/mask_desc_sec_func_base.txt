static
char* GetData(cmsIT8* it8, int nSet, int nField)
{
    TABLE* t = GetTable(it8);
    // Retrieves data from the specified row and column in the table.
    // Checks if the specified row (nSet) and column (nField) indices are valid.
    // <MASK>
    return t->Data [nSet * nSamples + nField];
}