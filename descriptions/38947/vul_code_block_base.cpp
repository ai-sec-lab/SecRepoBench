// get file format version and print to log
    ++it;
    
    if ((*it).tokens[0].empty()) {
        ASSIMP_LOG_ERROR("Invalid LWS file detectedm abort import.");
        return;
    }