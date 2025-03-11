unsigned int iBone  = 0;
    SkipSpacesAndLineEnd(currentChar,&currentChar);
    if ( !ParseUnsignedInt(currentChar,&currentChar,iBone) || !SkipSpaces(currentChar,&currentChar)) {
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