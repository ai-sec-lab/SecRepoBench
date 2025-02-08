if (token & 0x1) {
        // A run of bytes that are different than 0
        if (cbytes < -255) {
          // Runs can only encode a byte
          return -2;
        }
        uint8_t value = -cbytes;
        memset(_dest, value, (unsigned int) neblock);
      }
      nbytes = neblock;
      cbytes = 0;