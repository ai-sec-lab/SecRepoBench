void SMDImporter::ParseNodeInfo(const char* currentChar, const char** szCurrentOut) {
    unsigned int iBone  = 0;
    SkipSpacesAndLineEnd(currentChar,&currentChar);
    if ( !ParseUnsignedInt(currentChar,&currentChar,iBone) || !SkipSpaces(currentChar,&currentChar)) {
        throw DeadlyImportError("Unexpected EOF/EOL while parsing bone index");
    }
    if (iBone == UINT_MAX) {
        LogErrorNoThrow("Invalid bone number while parsing bone index");
        SMDI_PARSE_RETURN;
    }
    // add our bone to the list
    if (iBone >= asBones.size()) {
        asBones.resize(iBone+1);
    }
    SMD::Bone& bone = asBones[iBone];

    bool bQuota = true;
    if ('\"' != *currentChar) {
        LogWarning("Bone name is expected to be enclosed in "
            "double quotation marks. ");
        bQuota = false;
    } else {
        ++currentChar;
    }

    const char* szEnd = currentChar;
    for ( ;; ) {
        if (bQuota && '\"' == *szEnd) {
            iBone = (unsigned int)(szEnd - currentChar);
            ++szEnd;
            break;
        } else if (!bQuota && IsSpaceOrNewLine(*szEnd)) {
            iBone = (unsigned int)(szEnd - currentChar);
            break;
        } else if (!(*szEnd)) {
            LogErrorNoThrow("Unexpected EOF/EOL while parsing bone name");
            SMDI_PARSE_RETURN;
        }
        ++szEnd;
    }
    bone.mName = std::string(currentChar,iBone);
    currentChar = szEnd;

    // the only negative bone parent index that could occur is -1 AFAIK
    if(!ParseSignedInt(currentChar,&currentChar,(int&)bone.iParent))  {
        LogErrorNoThrow("Unexpected EOF/EOL while parsing bone parent index. Assuming -1");
        SMDI_PARSE_RETURN;
    }

    // go to the beginning of the next line
    SMDI_PARSE_RETURN;
}