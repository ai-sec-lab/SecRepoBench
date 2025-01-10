if (it == root.children.end() || (*it).tokens[0].empty()) {
        ASSIMP_LOG_ERROR("Invalid LWS file detectedm abort import.");
        return;
    }