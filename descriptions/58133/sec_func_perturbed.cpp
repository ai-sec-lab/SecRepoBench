int HashMgr::decode_flags(unsigned short** result, const std::string& flags, FileMgr* af) const {
  int len;
  if (flags.empty()) {
    *result = NULL;
    return 0;
  }
  switch (flag_mode) {
    case FLAG_LONG: {  // two-character flags (1x2yZz -> 1x 2y Zz)
      len = flags.size();
      if ((len & 1) == 1 && af != NULL)
        HUNSPELL_WARNING(stderr, "error: line %d: bad flagvector\n",
                         af->getlinenum());
      len >>= 1;
      *result = new unsigned short[len];
      for (int index = 0; index < len; index++) {
        unsigned short flag = ((unsigned short)((unsigned char)flags[index << 1]) << 8) |
                              ((unsigned short)((unsigned char)flags[(index << 1) | 1]));

        if (flag >= DEFAULTFLAGS && af != NULL) {
          HUNSPELL_WARNING(stderr,
                           "error: line %d: flag id %d is too large (max: %d)\n",
                           af->getlinenum(), flag, DEFAULTFLAGS - 1);
          flag = 0;
        }

        (*result)[index] = flag;
      }
      break;
    }
    case FLAG_NUM: {  // decimal numbers separated by comma (4521,23,233 -> 4521
                      // 23 233)
      len = int(1 + std::count_if(flags.begin(), flags.end(), [](char c) { return c == ','; }));
      *result = new unsigned short[len];
      unsigned short* dest = *result;
      const char* src = flags.c_str();
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
    }
    case FLAG_UNI: {  // UTF-8 characters
      std::vector<w_char> w;
      u8_u16(w, flags);
      len = w.size();
      *result = new unsigned short[len];
#if defined(__i386__) || defined(_M_IX86) || defined(_M_X64)
      memcpy(*result, w.data(), len * sizeof(unsigned short));
#else
      unsigned short* dest = *result;
      for (const w_char wc : w) {
        *dest = (unsigned short)wc;
        dest++;
      }
#endif
      break;
    }
    default: {  // Ispell's one-character flags (erfg -> e r f g)
      len = flags.size();
      *result = new unsigned short[len];
      unsigned short* dest = *result;
      for (const char flag : flags) {
        *dest = (unsigned char)flag;
        dest++;
      }
    }
  }
  return len;
}