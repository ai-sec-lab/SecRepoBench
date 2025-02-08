std::string PfxEntry::add(const char* word, size_t len) {
  std::string output;
  if ((len > strip.size() || (len == 0 && pmyMgr->get_fullstrip())) &&
      (len >= numconds) && test_condition(word) &&
      (!strip.size() ||
      (len >= strip.size() && strncmp(word, strip.c_str(), strip.size()) == 0))) {
    /* we have a match so add prefix */
    output.assign(appnd);
    output.append(word + strip.size());
  }
  return output;
}