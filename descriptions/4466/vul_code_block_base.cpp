{
            // Stacked text. Normal escapes don't work inside a stack

            pszInput += 2;
            while( *pszInput != ';' && *pszInput != '\0' )
            {
                if( pszInput[0] == '\\' &&
                    strchr( "^/#~", pszInput[1] ) != NULL )
                {
                    osMtextResult += pszInput[1];
                    pszInput++;
                }
                else if( strchr( "^/#~", pszInput[0] ) == NULL )
                {
                    osMtextResult += pszInput[0];
                }
                pszInput++;
            }
            if( pszInput[0] == ';' )
                pszInput++;
            if( pszInput[0] == '\0' )
                break;
        }
        else if( pszInput[0] == '\\'
                 && strchr( "\\{}", pszInput[1] ) != NULL )
        {
            // MTEXT character escapes

            osMtextResult += pszInput[1];
            pszInput++;
        }
        else
        {
            osMtextResult += *pszInput;
        }