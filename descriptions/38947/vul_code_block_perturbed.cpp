// get file format version and print to log
    ++elementiterator;
    
    if ((*elementiterator).tokens[0].empty()) {
        ASSIMP_LOG_ERROR("Invalid LWS file detectedm abort import.");
        return;
    }