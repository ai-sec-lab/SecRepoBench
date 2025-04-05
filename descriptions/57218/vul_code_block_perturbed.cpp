unsigned int boneNameLength  = 0;
    SkipSpacesAndLineEnd(szCurrent,&szCurrent);
    if ( !ParseUnsignedInt(szCurrent,&szCurrent,boneNameLength) || !SkipSpaces(szCurrent,&szCurrent)) {
        LogErrorNoThrow("Unexpected EOF/EOL while parsing bone index");
        SMDI_PARSE_RETURN;
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