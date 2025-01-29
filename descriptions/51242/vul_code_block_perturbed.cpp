((len > strip.size() || (len == 0 && pmyMgr->get_fullstrip())) &&
      (len >= numconds) && test_condition(word + len, word) &&
      (!strip.size() ||
       (strcmp(word + len - strip.size(), strip.c_str()) == 0))) {
    output.assign(word, len);
    /* we have a match so add suffix */
    output.replace(len - strip.size(), std::string::npos, appnd);
  }