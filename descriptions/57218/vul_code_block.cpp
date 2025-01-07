
    unsigned int iBone  = 0;
    SkipSpacesAndLineEnd(szCurrent,&szCurrent);
    if ( !ParseUnsignedInt(szCurrent,&szCurrent,iBone) || !SkipSpaces(szCurrent,&szCurrent)) {
        LogErrorNoThrow("Unexpected EOF/EOL while parsing bone index");
        SMDI_PARSE_RETURN;
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
            iBone = (unsigned int)(szEnd - szCurrent);
            ++szEnd;
            break;
        } else if (!bQuota && IsSpaceOrNewLine(*szEnd)) {
            iBone = (unsigned int)(szEnd - szCurrent);
            break;
        } else if (!(*szEnd)) {
            LogErrorNoThrow("Unexpected EOF/EOL while parsing bone name");
            SMDI_PARSE_RETURN;
        }
        ++szEnd;
    }
    bone.mName = std::string(szCurrent,iBone);
    szCurrent = szEnd;

    // the only negative bone parent index that could occur is -1 AFAIK
    if(!ParseSignedInt(szCurrent,&szCurrent,(int&)bone.iParent))  {
        LogErrorNoThrow("Unexpected EOF/EOL while parsing bone parent index. Assuming -1");
        SMDI_PARSE_RETURN;
    }

    // go to the beginning of the next line
    SMDI_PARSE_RETURN;
