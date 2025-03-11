ctbytes += 1;

      if (token & 0x1) {
        // A run of bytes that are different than 0
        if (cbytes < -255) {
          // Runs can only encode a byte
          return BLOSC2_ERROR_RUN_LENGTH;
        }
        uint8_t value = -cbytes;
        memset(comp_buffer, value, (unsigned int) neblock);
      }
      nbytes = neblock;
      cbytes = 0;  // everything is encoded in the cbytes token