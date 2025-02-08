unsigned short HashMgr::decode_flag(const std::string& flagstring) const {
  unsigned short s = 0;
  int i;
  switch (flag_mode) {
    case FLAG_LONG:
      if (flagstring.size() >= 2)
        s = ((unsigned short)((unsigned char)flagstring[0]) << 8) | ((unsigned short)((unsigned char)flagstring[1]));
      break;
    case FLAG_NUM:
      i = atoi(flagstring.c_str());
      if (i >= DEFAULTFLAGS) {
        HUNSPELL_WARNING(stderr, "error: flag id %d is too large (max: %d)\n",
                         i, DEFAULTFLAGS - 1);
        i = 0;
      }
      s = (unsigned short)i;
      break;
    case FLAG_UNI: {
      std::vector<w_char> w;
      u8_u16(w, flagstring);
      if (!w.empty())
        s = (unsigned short)w[0];
      break;
    }
    default:
      if (!flagstring.empty())
        s = (unsigned char)flagstring[0];
  }
  if (s == 0)
    HUNSPELL_WARNING(stderr, "error: 0 is wrong flag id\n");
  return s;
}