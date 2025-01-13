unsigned int iBone  = 0;
    SkipSpacesAndLineEnd(szCurrent,&szCurrent);
    if ( !ParseUnsignedInt(szCurrent,&szCurrent,iBone) || !SkipSpaces(szCurrent,&szCurrent)) {
        throw DeadlyImportError("Unexpected EOF/EOL while parsing bone index");
    }