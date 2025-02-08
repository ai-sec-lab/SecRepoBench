static
void *Type_Text_Description_Read(struct _cms_typehandler_struct* self, cmsIOHANDLER* io, cmsUInt32Number* nItems, cmsUInt32Number SizeOfTag)
{
    char* Text = NULL;
    wchar_t* UnicodeString = NULL;
    cmsMLU* mlu = NULL;
    cmsUInt32Number  AsciiCount;
    cmsUInt32Number  i, UnicodeCode, UnicodeCount;
    cmsUInt16Number  ScriptCodeCode, Dummy;
    cmsUInt8Number   ScriptCodeCount;

    *nItems = 0;

    //  One dword should be there
    if (SizeOfTag < sizeof(cmsUInt32Number)) return NULL;

    // Read len of ASCII
    if (!_cmsReadUInt32Number(io, &AsciiCount)) return NULL;
    SizeOfTag -= sizeof(cmsUInt32Number);

    // Check for size
    if (SizeOfTag < AsciiCount) return NULL;

    // All seems Ok, allocate the container
    mlu = cmsMLUalloc(self ->ContextID, 2);
    if (mlu == NULL) return NULL;

    // As many memory as size of tag
    Text = (char*) _cmsMalloc(self ->ContextID, AsciiCount + 1);
    if (Text == NULL) goto Error;

    // Read it
    if (io ->Read(io, Text, sizeof(char), AsciiCount) != AsciiCount) goto Error;
    SizeOfTag -= AsciiCount;

    // Make sure there is a terminator
    Text[AsciiCount] = 0;

    // Set the MLU entry. From here we can be tolerant to wrong types
    if (!cmsMLUsetASCII(mlu, cmsNoLanguage, cmsNoCountry, Text)) goto Error;
    _cmsFree(self ->ContextID, (void*) Text);
    Text = NULL;

    // Skip Unicode code
    if (SizeOfTag < 2* sizeof(cmsUInt32Number)) goto Done;
    if (!_cmsReadUInt32Number(io, &UnicodeCode)) goto Done;
    if (!_cmsReadUInt32Number(io, &UnicodeCount)) goto Done;
    SizeOfTag -= 2* sizeof(cmsUInt32Number);

    // <MASK>
    if (Text) _cmsFree(self ->ContextID, (void*) Text);
    if (mlu) cmsMLUfree(mlu);
    return NULL;
}