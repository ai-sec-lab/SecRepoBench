if (len == 128 + 1) return EXR_ERR_CORRUPT_CHUNK;
        // account for extra byte for nil terminator
        len += 1;