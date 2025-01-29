int num = -1;
  int i = 0;
  int np = 0;
  std::string::const_iterator iter = line.begin();
  std::string::const_iterator start_piece = mystrsep(line, iter);
  while (start_piece != line.end()) {
    switch (i) {
      case 0: {
        np++;
        break;
      }
      case 1: {
        num = atoi(std::string(start_piece, iter).c_str());
        if (num < 1) {
          HUNSPELL_WARNING(stderr, "error: line %d: bad entry number\n",
                           fileMgr->getlinenum());
          return false;
        }
        phone = new phonetable;
        phone->utf8 = (char)utf8;
        np++;
        break;
      }
      default:
        break;
    }
    ++i;
    start_piece = mystrsep(line, iter);
  }
  if (np != 2) {
    HUNSPELL_WARNING(stderr, "error: line %d: missing data\n",
                     fileMgr->getlinenum());
    return false;
  }

  /* now parse the phone->num lines to read in the remainder of the table */
  for (int j = 0; j < num; ++j) {
    std::string nl;
    if (!fileMgr->getline(nl))
      return false;
    mychomp(nl);
    i = 0;
    const size_t old_size = phone->rules.size();
    iter = nl.begin();
    start_piece = mystrsep(nl, iter);
    while (start_piece != nl.end()) {
      {
        switch (i) {
          case 0: {
            if (nl.compare(start_piece - nl.begin(), 5, "PHONE", 5) != 0) {
              HUNSPELL_WARNING(stderr, "error: line %d: table is corrupt\n",
                               fileMgr->getlinenum());
              return false;
            }
            break;
          }
          case 1: {
            phone->rules.push_back(std::string(start_piece, iter));
            break;
          }
          case 2: {
            phone->rules.push_back(std::string(start_piece, iter));
            mystrrep(phone->rules.back(), "_", "");
            break;
          }
          default:
            break;
        }
        ++i;
      }
      start_piece = mystrsep(nl, iter);
    }
    if (phone->rules.size() != old_size + 2) {
      HUNSPELL_WARNING(stderr, "error: line %d: table is corrupt\n",
                       fileMgr->getlinenum());
      phone->rules.clear();
      return false;
    }
  }
  phone->rules.push_back("");
  phone->rules.push_back("");
  init_phonet_hash(*phone);