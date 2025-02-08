int HashMgr::add_word(const std::string& in_word,
                      int wcl,
                      unsigned short* aff,
                      int al,
                      const std::string* in_desc,
                      bool onlyupcase,
                      int captype) {
  const std::string* word = &in_word;
  const std::string* desc = in_desc;

  std::string *word_copy = NULL;
  std::string *desc_copy = NULL;
  if ((!ignorechars.empty() && !has_no_ignored_chars(in_word, ignorechars)) || complexprefixes) {
    word_copy = new std::string(in_word);

    if (!ignorechars.empty()) {
      if (utf8) {
        wcl = remove_ignored_chars_utf(*word_copy, ignorechars_utf16);
      } else {
        remove_ignored_chars(*word_copy, ignorechars);
      }
    }

    if (complexprefixes) {
      if (utf8)
        wcl = reverseword_utf(*word_copy);
      else
        reverseword(*word_copy);

      if (in_desc && aliasm.empty()) {
        desc_copy = new std::string(*in_desc);

        if (complexprefixes) {
          if (utf8)
            reverseword_utf(*desc_copy);
          else
            reverseword(*desc_copy);
        }
        desc = desc_copy;
      }
    }

    word = word_copy;
  }

  // limit of hp->blen
  if (word->size() > std::numeric_limits<unsigned char>::max()) {
    HUNSPELL_WARNING(stderr, "error: word len %ld is over max limit\n", word->size());
    delete desc_copy;
    delete word_copy;
    free_flag(aff, al);
    return 1;
  }

  bool upcasehomonym = false;
  int descl = desc ? (!aliasm.empty() ? sizeof(char*) : desc->size() + 1) : 0;
  // variable-length hash record with word and optional fields
  struct hentry* hp =
      (struct hentry*)malloc(sizeof(struct hentry) + word->size() + descl);
  if (!hp) {
    delete desc_copy;
    delete word_copy;
    free_flag(aff, al);
    return 1;
  }

  char* hpw = hp->word;
  strcpy(hpw, word->c_str());

  int i = hash(hpw);

  hp->blen = (unsigned char)word->size();
  hp->clen = (unsigned char)wcl;
  hp->alen = (short)al;
  hp->astr = aff;
  hp->next = NULL;
  hp->next_homonym = NULL;
  hp->var = (captype == INITCAP) ? H_OPT_INITCAP : 0;

  // store the description string or its pointer
  if (desc) {
    hp->var |= H_OPT;
    if (!aliasm.empty()) {
      hp->var |= H_OPT_ALIASM;
      store_pointer(hpw + word->size() + 1, get_aliasm(atoi(desc->c_str())));
    } else {
      strcpy(hpw + word->size() + 1, desc->c_str());
    }
    if (HENTRY_FIND(hp, MORPH_PHON)) {
      hp->var |= H_OPT_PHON;
      // store ph: fields (pronounciation, misspellings, old orthography etc.)
      // of a morphological description in reptable to use in REP replacements.
      size_t predicted = tableptr.size() / MORPH_PHON_RATIO;
      if (reptable.capacity() < predicted)
          reptable.reserve(predicted);
      std::string fields = HENTRY_DATA(hp);
      std::string::const_iterator iter = fields.begin();
      std::string::const_iterator start_piece = mystrsep(fields, iter);
      while (start_piece != fields.end()) {
        if (std::string(start_piece, iter).find(MORPH_PHON) == 0) {
          std::string ph = std::string(start_piece, iter).substr(sizeof MORPH_PHON - 1);
          if (ph.size() > 0) {
            std::vector<w_char> w;
            size_t strippatt;
            std::string wordpart;
            // dictionary based REP replacement, separated by "->"
            // for example "pretty ph:prity ph:priti->pretti" to handle
            // both prity -> pretty and pritier -> prettiest suggestions.
            if (((strippatt = ph.find("->")) != std::string::npos) &&
                    (strippatt > 0) && (strippatt < ph.size() - 2)) {
                wordpart = ph.substr(strippatt + 2);
                ph.erase(ph.begin() + strippatt, ph.end());
            } else
                wordpart = in_word;
            // when the ph: field ends with the character *,
            // strip last character of the pattern and the replacement
            // to match in REP suggestions also at character changes,
            // for example, "pretty ph:prity*" results "prit->prett"
            // REP replacement instead of "prity->pretty", to get
            // prity->pretty and pritiest->prettiest suggestions.
            // <MASK>
          }
        }
        start_piece = mystrsep(fields, iter);
      }
    }
  }

  struct hentry* dp = tableptr[i];
  if (!dp) {
    tableptr[i] = hp;
    delete desc_copy;
    delete word_copy;
    return 0;
  }
  while (dp->next != NULL) {
    if ((!dp->next_homonym) && (strcmp(hp->word, dp->word) == 0)) {
      // remove hidden onlyupcase homonym
      if (!onlyupcase) {
        if ((dp->astr) && TESTAFF(dp->astr, ONLYUPCASEFLAG, dp->alen)) {
          delete[] dp->astr;
          dp->astr = hp->astr;
          dp->alen = hp->alen;
          free(hp);
          delete desc_copy;
          delete word_copy;
          return 0;
        } else {
          dp->next_homonym = hp;
        }
      } else {
        upcasehomonym = true;
      }
    }
    dp = dp->next;
  }
  if (strcmp(hp->word, dp->word) == 0) {
    // remove hidden onlyupcase homonym
    if (!onlyupcase) {
      if ((dp->astr) && TESTAFF(dp->astr, ONLYUPCASEFLAG, dp->alen)) {
        delete[] dp->astr;
        dp->astr = hp->astr;
        dp->alen = hp->alen;
        free(hp);
        delete desc_copy;
        delete word_copy;
        return 0;
      } else {
        dp->next_homonym = hp;
      }
    } else {
      upcasehomonym = true;
    }
  }
  if (!upcasehomonym) {
    dp->next = hp;
  } else {
    // remove hidden onlyupcase homonym
    delete[] hp->astr;
    free(hp);
  }

  delete desc_copy;
  delete word_copy;
  return 0;
}