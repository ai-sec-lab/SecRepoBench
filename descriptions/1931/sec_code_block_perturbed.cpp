if( nCoordWidth <= 0 )
        nCoordWidth = 10;

    nZWidth = atoi(poRecord->GetField(31,35));                // ZLEN
    if( nZWidth <= 0 )
        nZWidth = 10;