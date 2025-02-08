(strEQc (f->type, "TFv"))
        {
          char *str = (char *)calloc (f->size, 1);
          strncpy (str, *(char**)val, f->size);
          memcpy (old, &str, sizeof (char*)); // size of ptr
        }
      // ascii