while( dparms.size() < 18 )
    {
        dparms.push_back(0.0);
    }

    dparms.resize(18);

    // This is rather iffy!
    if( STARTS_WITH_CI(geosys.c_str(),"DEG" /* "DEGREE" */) )
        dparms[17] = (double) (int) UNIT_DEGREE;
    else if( STARTS_WITH_CI(geosys.c_str(), "MET") )
        dparms[17] = (double) (int) UNIT_METER;
    else if( STARTS_WITH_CI(geosys.c_str(), "FOOT") )
        dparms[17] = (double) (int) UNIT_US_FOOT;
    else if( STARTS_WITH_CI(geosys.c_str(), "FEET") )
        dparms[17] = (double) (int) UNIT_US_FOOT;
    else if( STARTS_WITH_CI(geosys.c_str(),"INTL " /* "INTL FOOT" */) )
        dparms[17] = (double) (int) UNIT_INTL_FOOT;
    else if( STARTS_WITH_CI(geosys.c_str(), "SPCS") )
        dparms[17] = (double) (int) UNIT_METER;
    else if( STARTS_WITH_CI(geosys.c_str(), "SPIF") )
        dparms[17] = (double) (int) UNIT_INTL_FOOT;
    else if( STARTS_WITH_CI(geosys.c_str(), "SPAF") )
        dparms[17] = (double) (int) UNIT_US_FOOT;
    else
        dparms[17] = -1.0; /* unknown */

    return dparms;