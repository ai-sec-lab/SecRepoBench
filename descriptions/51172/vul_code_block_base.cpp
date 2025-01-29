for (const char* p = src; *p; p++) {
        if (*p == ',') {
          int i = atoi(src);
          if (i >= DEFAULTFLAGS)
            HUNSPELL_WARNING(
                stderr, "error: line %d: flag id %d is too large (max: %d)\n",
                af->getlinenum(), i, DEFAULTFLAGS - 1);
          *dest = (unsigned short)i;
          if (*dest == 0)
            HUNSPELL_WARNING(stderr, "error: line %d: 0 is wrong flag id\n",
                             af->getlinenum());
          src = p + 1;
          dest++;
        }
      }
      int i = atoi(src);
      if (i >= DEFAULTFLAGS)
        HUNSPELL_WARNING(stderr,
                         "error: line %d: flag id %d is too large (max: %d)\n",
                         af->getlinenum(), i, DEFAULTFLAGS - 1);
      *dest = (unsigned short)i;
      if (*dest == 0)
        HUNSPELL_WARNING(stderr, "error: line %d: 0 is wrong flag id\n",
                         af->getlinenum());
      break;