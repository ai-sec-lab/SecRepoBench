while( projectionparameters.size() < 18 )
    {
        projectionparameters.push_back(0.0);
    }

    projectionparameters.resize(18);

    // This is rather iffy!
    if( STARTS_WITH_CI(geosys.c_str(),"DEG" /* "DEGREE" */) )
        projectionparameters[17] = (double) (int) UNIT_DEGREE;
    else if( STARTS_WITH_CI(geosys.c_str(), "MET") )
        projectionparameters[17] = (double) (int) UNIT_METER;
    else if( STARTS_WITH_CI(geosys.c_str(), "FOOT") )
        projectionparameters[17] = (double) (int) UNIT_US_FOOT;
    else if( STARTS_WITH_CI(geosys.c_str(), "FEET") )
        projectionparameters[17] = (double) (int) UNIT_US_FOOT;
    else if( STARTS_WITH_CI(geosys.c_str(),"INTL " /* "INTL FOOT" */) )
        projectionparameters[17] = (double) (int) UNIT_INTL_FOOT;
    else if( STARTS_WITH_CI(geosys.c_str(), "SPCS") )
        projectionparameters[17] = (double) (int) UNIT_METER;
    else if( STARTS_WITH_CI(geosys.c_str(), "SPIF") )
        projectionparameters[17] = (double) (int) UNIT_INTL_FOOT;
    else if( STARTS_WITH_CI(geosys.c_str(), "SPAF") )
        projectionparameters[17] = (double) (int) UNIT_US_FOOT;
    else
        projectionparameters[17] = -1.0; /* unknown */

    return projectionparameters;