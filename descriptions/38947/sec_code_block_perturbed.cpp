// get file format version and print to log
    ++elementiterator;

    if (elementiterator == root.children.end() || (*elementiterator).tokens[0].empty()) {
        ASSIMP_LOG_ERROR("Invalid LWS file detectedm abort import.");
        return;
    }