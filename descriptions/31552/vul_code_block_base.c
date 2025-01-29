{
        // A run of bytes that are different than 0
        if (cbytes < -255) {
          // Runs can only encode a byte
          return BLOSC2_ERROR_RUN_LENGTH;
        }
        uint8_t value = -cbytes;
        memset(_dest, value, (unsigned int) neblock);
      }