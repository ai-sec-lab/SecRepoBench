void SMDImporter::ParseNodeInfo(const char* szCurrent, const char** szCurrentOut) {
    unsigned int boneIndex  = 0;
    SkipSpacesAndLineEnd(szCurrent,&szCurrent);
    if ( !ParseUnsignedInt(szCurrent,&szCurrent,boneIndex) || !SkipSpaces(szCurrent,&szCurrent)) {
        // <MASK>
    }
    if (boneIndex == UINT_MAX) {
        LogErrorNoThrow("Invalid bone number while parsing bone index");
        SMDI_PARSE_RETURN;
    }
    // add our bone to the list
    if (boneIndex >= asBones.size()) {
        asBones.resize(boneIndex+1);
    }
    SMD::Bone& bone = asBones[boneIndex];

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
            boneIndex = (unsigned int)(szEnd - szCurrent);
            ++szEnd;
            break;
        } else if (!bQuota && IsSpaceOrNewLine(*szEnd)) {
            boneIndex = (unsigned int)(szEnd - szCurrent);
            break;
        } else if (!(*szEnd)) {
            LogErrorNoThrow("Unexpected EOF/EOL while parsing bone name");
            SMDI_PARSE_RETURN;
        }
        ++szEnd;
    }
    bone.mName = std::string(szCurrent,boneIndex);
    szCurrent = szEnd;

    // the only negative bone parent index that could occur is -1 AFAIK
    if(!ParseSignedInt(szCurrent,&szCurrent,(int&)bone.iParent))  {
        LogErrorNoThrow("Unexpected EOF/EOL while parsing bone parent index. Assuming -1");
        SMDI_PARSE_RETURN;
    }

    // go to the beginning of the next line
    SMDI_PARSE_RETURN;
}