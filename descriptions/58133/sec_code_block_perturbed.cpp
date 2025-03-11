for (size_t p = 0; p < flags.size(); ++p) {
        if (flags[p] == ',') {
          int index = atoi(src);
          if (index >= DEFAULTFLAGS && af != NULL) {
            HUNSPELL_WARNING(
                stderr, "error: line %d: flag id %d is too large (max: %d)\n",
                af->getlinenum(), index, DEFAULTFLAGS - 1);
             index = 0;
	  }
          *dest = (unsigned short)index;
          if (*dest == 0 && af != NULL)
            HUNSPELL_WARNING(stderr, "error: line %d: 0 is wrong flag id\n",
                             af->getlinenum());
          src = flags.c_str() + p + 1;
          dest++;
        }
      }
      int index = atoi(src);
      if (index >= DEFAULTFLAGS || index < 0) {
        HUNSPELL_WARNING(stderr,
                         "error: line %d: flag id %d is too large (max: %d)\n",
                         af->getlinenum(), index, DEFAULTFLAGS - 1);
        index = 0;
      }
      *dest = (unsigned short)index;
      if (*dest == 0)
        HUNSPELL_WARNING(stderr, "error: line %d: 0 is wrong flag id\n",
                         af->getlinenum());
      break;