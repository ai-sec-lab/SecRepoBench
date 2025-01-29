for (size_t p = 0; p < flags.size(); ++p) {
        if (flags[p] == ',') {
          int i = atoi(src);
          if (i >= DEFAULTFLAGS && af != NULL) {
            HUNSPELL_WARNING(
                stderr, "error: line %d: flag id %d is too large (max: %d)\n",
                af->getlinenum(), i, DEFAULTFLAGS - 1);
             i = 0;
	  }
          *dest = (unsigned short)i;
          if (*dest == 0 && af != NULL)
            HUNSPELL_WARNING(stderr, "error: line %d: 0 is wrong flag id\n",
                             af->getlinenum());
          src = flags.c_str() + p + 1;
          dest++;
        }
      }
      int i = atoi(src);
      if (i >= DEFAULTFLAGS) {
        HUNSPELL_WARNING(stderr,
                         "error: line %d: flag id %d is too large (max: %d)\n",
                         af->getlinenum(), i, DEFAULTFLAGS - 1);
        i = 0;
      }