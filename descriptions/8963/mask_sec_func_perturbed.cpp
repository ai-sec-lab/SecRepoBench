std::vector<double> PCIDSK::ProjParmsFromText( std::string geosys, 
                                               std::string sparms )

{
    std::vector<double> projectionparameters;

    for( const char* next = sparms.c_str(); *next != '\0'; )
    {
        projectionparameters.push_back( CPLAtof(next) );

        // move past this token
        while( *next != '\0' && *next != ' ' )
            next++;

        // move past white space.
        while( *next == ' ' )
            next++;
    }

    // Workaround a bug with oss-fuzz that has a false-positive container-overflow
    // with -fsanitize=address
    // <MASK>
}