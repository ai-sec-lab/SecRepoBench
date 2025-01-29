if( !bOK )
        {
            for( i = 0; i < 4; i++ )
            {
                CPLFree( psImage->apanVQLUT[i] );
                psImage->apanVQLUT[i] = NULL;
            }
            return FALSE;
        }