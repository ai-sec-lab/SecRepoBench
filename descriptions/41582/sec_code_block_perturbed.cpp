char buff[4] = {0};
        memcpy(buff, head+6, 3);
        ASSIMP_LOG_WARN( "Unrecognized nendo file format version, continuing happily ... :", buff);