void SMDImporter::ParseNodeInfo(const char* szCurrent, const char** szCurrentOut) {
    unsigned int boneNameLength  = 0;
    SkipSpacesAndLineEnd(szCurrent,&szCurrent);
    if ( !ParseUnsignedInt(szCurrent,&szCurrent,boneNameLength) || !SkipSpaces(szCurrent,&szCurrent)) {
        throw DeadlyImportError("Unexpected EOF/EOL while parsing bone index");
    }
    if (boneNameLength == UINT_MAX) {
        LogErrorNoThrow("Invalid bone number while parsing bone index");
        SMDI_PARSE_RETURN;
    }
    // add our bone to the list
    if (boneNameLength >= asBones.size()) {
        asBones.resize(boneNameLength+1);
    }
    SMD::Bone& bone = asBones[boneNameLength];

    bool bQuota = true;
    if ('\"' != *szCurrent) {
        LogWarning("Bone name is expected to be enclosed in "
            "double quotation marks. ");
        bQuota = false;
    } else {
        ++szCurrent;
    }

    const char* szEnd = szCurrent;
    for ( ;; ) {
        if (bQuota && '\"' == *szEnd) {
            boneNameLength = (unsigned int)(szEnd - szCurrent);
            ++szEnd;
            break;
        } else if (!bQuota && IsSpaceOrNewLine(*szEnd)) {
            boneNameLength = (unsigned int)(szEnd - szCurrent);
            break;
        } else if (!(*szEnd)) {
            LogErrorNoThrow("Unexpected EOF/EOL while parsing bone name");
            SMDI_PARSE_RETURN;
        }
        ++szEnd;
    }
    bone.mName = std::string(szCurrent,boneNameLength);
    szCurrent = szEnd;

    // the only negative bone parent index that could occur is -1 AFAIK
    if(!ParseSignedInt(szCurrent,&szCurrent,(int&)bone.iParent))  {
        LogErrorNoThrow("Unexpected EOF/EOL while parsing bone parent index. Assuming -1");
        SMDI_PARSE_RETURN;
    }

    // go to the beginning of the next line
    SMDI_PARSE_RETURN;
}