
          if (blob_offset + *blob_offset >= pe->data + pe->data_size)
          {
            row_ptr += row_size;
            continue;
          }

          blob_offset += 1;

          if (*blob_offset == 0xFF || *blob_offset == 0x00)
          {
            typelib[0] = '\0';
          }
          else
          {
            strncpy(typelib, (char*) blob_offset, MAX_TYPELIB_SIZE);
            typelib[MAX_TYPELIB_SIZE] = '\0';
          }