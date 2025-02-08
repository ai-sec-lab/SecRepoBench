std::string SfxEntry::add(const char* word, size_t len) {
  std::string result;
  /* make sure all conditions match */
  if ((len > strip.size() || (len == 0 && pmyMgr->get_fullstrip())) &&
      (len >= numconds) && test_condition(word + len, word) &&
      (!strip.size() ||
       (len >= strip.size() && strcmp(word + len - strip.size(), strip.c_str()) == 0))) {
    result.assign(word, len);
    /* we have a match so add suffix */
    result.replace(len - strip.size(), std::string::npos, appnd);
  }
  return result;
}