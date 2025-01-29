std::unique_ptr<phonetable> new_phone;
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
                           af->getlinenum());
          return false;
        }
        new_phone.reset(new phonetable);
        new_phone->utf8 = (char)utf8;
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
                     af->getlinenum());
    return false;
  }

  /* now parse the phone->num lines to read in the remainder of the table */
  for (int j = 0; j < num; ++j) {
    std::string nl;
    if (!af->getline(nl))
      return false;
    mychomp(nl);
    i = 0;
    const size_t old_size = new_phone->rules.size();
    iter = nl.begin();
    start_piece = mystrsep(nl, iter);
    while (start_piece != nl.end()) {
      {
        switch (i) {
          case 0: {
            if (nl.compare(start_piece - nl.begin(), 5, "PHONE", 5) != 0) {
              HUNSPELL_WARNING(stderr, "error: line %d: table is corrupt\n",
                               af->getlinenum());
              return false;
            }
            break;
          }
          case 1: {
            new_phone->rules.push_back(std::string(start_piece, iter));
            break;
          }
          case 2: {
            new_phone->rules.push_back(std::string(start_piece, iter));
            mystrrep(new_phone->rules.back(), "_", "");
            break;
          }
          default:
            break;
        }
        ++i;
      }
      start_piece = mystrsep(nl, iter);
    }
    if (new_phone->rules.size() != old_size + 2) {
      HUNSPELL_WARNING(stderr, "error: line %d: table is corrupt\n",
                       af->getlinenum());
      return false;
    }
  }
  new_phone->rules.push_back("");
  new_phone->rules.push_back("");
  init_phonet_hash(*new_phone);
  phone = new_phone.release();