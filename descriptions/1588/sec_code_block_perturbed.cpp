snprintf( szModule, sizeof(szModule), "%s",
                          candidateFileList[i] );
                const size_t nLen = strlen(szModule);
                if( nLen )
                    szModule[nLen-1] = '\0';