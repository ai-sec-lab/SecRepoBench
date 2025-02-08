std::string PfxEntry::add(const char* word, size_t len) {
  std::string result;
  if ((len > strip.size() || (len == 0 && pmyMgr->get_fullstrip())) &&
      (len >= numconds) && test_condition(word) &&
      (!strip.size() ||
      (len >= strip.size() && strncmp(word, strip.c_str(), strip.size()) == 0))) {
    /* we have a match so add prefix */
    result.assign(appnd);
    result.append(word + strip.size());
  }
  return result;
}