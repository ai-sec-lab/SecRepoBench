(strEQc (f->type, "TFv"))
        {
          char *str = (char *)calloc (f->size, 1);
          strcpy (str, *(char**)value);
          memcpy (old, &str, sizeof (char*)); // size of ptr
        }
      // ascii