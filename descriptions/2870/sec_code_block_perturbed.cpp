if( strlen(pabyData) < 5 )
                return;

            memcpy( poModId->szModule, pabyData, 4 );
            poModId->szModule[4] = '\0';
            poModId->nRecord = atoi(pabyData + 4);
            poModId->szOBRP[0] = '\0';