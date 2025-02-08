CPLString ACTextUnescape( const char *pszRawInput, const char *pszEncoding,
    bool bIsMText )

{
    CPLString osResult;
    CPLString osInput = pszRawInput;

/* -------------------------------------------------------------------- */
/*      Translate text from Win-1252 to UTF8.  We approximate this      */
/*      by treating Win-1252 as Latin-1.  Note that we likely ought     */
/*      to be consulting the $DWGCODEPAGE header variable which         */
/*      defaults to ANSI_1252 if not set.                               */
/* -------------------------------------------------------------------- */
    osInput.Recode( pszEncoding, CPL_ENC_UTF8 );

    const char *pszInput = osInput.c_str();

/* -------------------------------------------------------------------- */
/*      Now translate low-level escape sequences.  They are all plain   */
/*      ASCII characters and won't have been affected by the UTF8       */
/*      recoding.                                                       */
/* -------------------------------------------------------------------- */
    while( *pszInput != '\0' )
    {
        if( pszInput[0] == '^' && pszInput[1] != '\0' )
        {
            if( pszInput[1] == ' ' )
                osResult += '^';
            else
                osResult += static_cast<char>( toupper(pszInput[1]) ) ^ 0x40;
            pszInput++;
        }
        else if( STARTS_WITH_CI(pszInput, "%%c")
            || STARTS_WITH_CI(pszInput, "%%d")
            || STARTS_WITH_CI(pszInput, "%%p") )
        {
            wchar_t anWCharString[2];

            anWCharString[1] = 0;

            // These are special symbol representations for AutoCAD.
            if( STARTS_WITH_CI(pszInput, "%%c") )
                anWCharString[0] = 0x2300; // diameter (0x00F8 is a good approx)
            else if( STARTS_WITH_CI(pszInput, "%%d") )
                anWCharString[0] = 0x00B0; // degree
            else if( STARTS_WITH_CI(pszInput, "%%p") )
                anWCharString[0] = 0x00B1; // plus/minus

            char *pszUTF8Char = CPLRecodeFromWChar( anWCharString,
                CPL_ENC_UCS2,
                CPL_ENC_UTF8 );

            osResult += pszUTF8Char;
            CPLFree( pszUTF8Char );

            pszInput += 2;
        }
        else if( !bIsMText && ( STARTS_WITH_CI(pszInput, "%%u")
            || STARTS_WITH_CI(pszInput, "%%o") ) )
        {
            // Underline and overline markers. These have no effect in MTEXT
            pszInput += 2;
        }
        else
        {
            osResult += pszInput[0];
        }

        pszInput++;
    }

    if( !bIsMText )
        return osResult;

/* -------------------------------------------------------------------- */
/*      If this is MTEXT, or something similar (e.g. DIMENSION text),   */
/*      do a second pass to strip additional MTEXT format codes.        */
/* -------------------------------------------------------------------- */
    pszInput = osResult.c_str();
    CPLString osMtextResult;

    while( *pszInput != '\0' )
    {
        if( pszInput[0] == '\\' && pszInput[1] == 'P' )
        {
            osMtextResult += '\n';
            pszInput++;
        }
        else if( pszInput[0] == '\\' && pszInput[1] == '~' )
        {
            osMtextResult += ' ';
            pszInput++;
        }
        else if( pszInput[0] == '\\' && pszInput[1] == 'U'
                 && pszInput[2] == '+' && CPLStrnlen(pszInput, 7) >= 7 )
        {
            CPLString osHex;
            unsigned int iChar = 0;

            osHex.assign( pszInput+3, 4 );
            sscanf( osHex.c_str(), "%x", &iChar );

            wchar_t anWCharString[2];
            anWCharString[0] = (wchar_t) iChar;
            anWCharString[1] = 0;

            char *pszUTF8Char = CPLRecodeFromWChar( anWCharString,
                                                    CPL_ENC_UCS2,
                                                    CPL_ENC_UTF8 );

            osMtextResult += pszUTF8Char;
            CPLFree( pszUTF8Char );

            pszInput += 6;
        }
        else if( pszInput[0] == '{' || pszInput[0] == '}' )
        {
            // Skip braces, which are used for grouping
        }
        else if( pszInput[0] == '\\'
                 && strchr( "WTAHFfCcQp", pszInput[1] ) != NULL )
        {
            // eg. \W1.073172x;\T1.099;Bonneuil de Verrines
            // See data/dwg/EP/42002.dwg
            // These are all inline formatting codes which take an argument
            // up to the first semicolon (\W for width, \f for font, etc)

            while( *pszInput != ';' && *pszInput != '\0' )
                pszInput++;
            if( *pszInput == '\0' )
                break;
        }
        else if( pszInput[0] == '\\'
                && strchr( "KkLlOo", pszInput[1] ) != NULL )
        {
            // Inline formatting codes that don't take an argument

            pszInput++;
        }
        else if( pszInput[0] == '\\' && pszInput[1] == 'S' )
        // <MASK>

        pszInput++;
    }

    return osMtextResult;
}