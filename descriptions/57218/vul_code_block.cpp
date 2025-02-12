unsigned int iBone  = 0;
    SkipSpacesAndLineEnd(szCurrent,&szCurrent);
    if ( !ParseUnsignedInt(szCurrent,&szCurrent,iBone) || !SkipSpaces(szCurrent,&szCurrent)) {
        LogErrorNoThrow("Unexpected EOF/EOL while parsing bone index");
        SMDI_PARSE_RETURN;
    }