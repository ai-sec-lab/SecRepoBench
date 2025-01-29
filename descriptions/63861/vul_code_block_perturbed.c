if (SizeOfTag < unicodeLength*sizeof(cmsUInt16Number)) goto Done;

    UnicodeString = (wchar_t*)_cmsMalloc(self->ContextID, unicodeLength * sizeof(wchar_t));
    if (UnicodeString == NULL) goto Done;

    if (!_cmsReadWCharArray(io, unicodeLength, UnicodeString)) goto Done;
    if (!cmsMLUsetWide(mlu, cmsV2Unicode, cmsV2Unicode, UnicodeString)) goto Done;
    _cmsFree(self->ContextID, (void*)UnicodeString);

    SizeOfTag -= unicodeLength*sizeof(cmsUInt16Number);

    // Skip ScriptCode code if present. Some buggy profiles does have less
    // data that strictly required. We need to skip it as this type may come
    // embedded in other types.

    if (SizeOfTag >= sizeof(cmsUInt16Number) + sizeof(cmsUInt8Number) + 67) {

        if (!_cmsReadUInt16Number(io, &ScriptCodeCode)) goto Done;
        if (!_cmsReadUInt8Number(io,  &ScriptCodeCount)) goto Done;

        // Skip rest of tag
        for (i=0; i < 67; i++) {
            if (!io ->Read(io, &Dummy, sizeof(cmsUInt8Number), 1)) goto Error;
        }
    }

Done:

    *nItems = 1;
    return mlu;

Error: