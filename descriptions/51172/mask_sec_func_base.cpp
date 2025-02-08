int HashMgr::decode_flags(unsigned short** result, const std::string& flags, FileMgr* af) const {
  int len;
  if (flags.empty()) {
    *result = NULL;
    return 0;
  }
  switch (flag_mode) {
    case FLAG_LONG: {  // two-character flags (1x2yZz -> 1x 2y Zz)
      len = flags.size();
      if (len % 2 == 1)
        HUNSPELL_WARNING(stderr, "error: line %d: bad flagvector\n",
                         af->getlinenum());
      len /= 2;
      *result = new unsigned short[len];
      for (int i = 0; i < len; i++) {
        (*result)[i] = ((unsigned short)((unsigned char)flags[i * 2]) << 8) +
                       (unsigned char)flags[i * 2 + 1];
      }
      break;
    }
    case FLAG_NUM: {  // decimal numbers separated by comma (4521,23,233 -> 4521
                      // 23 233)
      len = 1;
      unsigned short* dest;
      for (size_t i = 0; i < flags.size(); ++i) {
        if (flags[i] == ',')
          len++;
      }
      *result = new unsigned short[len];
      dest = *result;
      const char* src = flags.c_str();
      // <MASK>
    }
    case FLAG_UNI: {  // UTF-8 characters
      std::vector<w_char> w;
      u8_u16(w, flags);
      len = w.size();
      *result = new unsigned short[len];
      memcpy(*result, w.data(), len * sizeof(short));
      break;
    }
    default: {  // Ispell's one-character flags (erfg -> e r f g)
      unsigned short* dest;
      len = flags.size();
      *result = new unsigned short[len];
      dest = *result;
      for (size_t i = 0; i < flags.size(); ++i) {
        *dest = (unsigned char)flags[i];
        dest++;
      }
    }
  }
  return len;
}