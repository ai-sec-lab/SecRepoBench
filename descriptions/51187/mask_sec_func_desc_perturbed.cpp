int HashMgr::add_word(const std::string& in_word,
                      int wcl,
                      unsigned short* aff,
                      int al,
                      const std::string* inputdesc,
                      bool onlyupcase,
                      int captype) {

  // If the word contains ignored characters or complex prefixes are being handled:
  //   - Create a new copy of the word and remove ignored characters if necessary.
  //   - Reverse the word if complex prefixes are enabled.
  //   - If a description is provided and aliasing is not used, create a copy and reverse it if complex prefixes are enabled.
  // Update the word and description pointers to point to the new copies.
  // <MASK>

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
  memcpy(hpw, word->data(), word->size());
  hpw[word->size()] = 0;

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
            if (ph.at(ph.size()-1) == '*') {
              strippatt = 1;
              size_t stripword = 0;
              if (utf8) {
                while ((strippatt < ph.size()) &&
                  ((ph.at(ph.size()-strippatt-1) & 0xc0) == 0x80))
                     ++strippatt;
                while ((stripword < wordpart.size()) &&
                  ((wordpart.at(wordpart.size()-stripword-1) & 0xc0) == 0x80))
                     ++stripword;
              }
              ++strippatt;
              ++stripword;
              if ((ph.size() > strippatt) && (wordpart.size() > stripword)) {
                ph.erase(ph.size()-strippatt, strippatt);
                wordpart.erase(wordpart.size()-stripword, stripword);
              }
            }
            // capitalize lowercase pattern for capitalized words to support
            // good suggestions also for capitalized misspellings, eg.
            // Wednesday ph:wendsay
            // results wendsay -> Wednesday and Wendsay -> Wednesday, too.
            if (captype==INITCAP) {
              std::string ph_capitalized;
              if (utf8) {
                u8_u16(w, ph);
                if (get_captype_utf8(w, langnum) == NOCAP) {
                  mkinitcap_utf(w, langnum);
                  u16_u8(ph_capitalized, w);
                }
              } else if (get_captype(ph, csconv) == NOCAP)
                  mkinitcap(ph_capitalized, csconv);

              if (ph_capitalized.size() > 0) {
                // add also lowercase word in the case of German or
                // Hungarian to support lowercase suggestions lowercased by
                // compound word generation or derivational suffixes
                // (for example by adjectival suffix "-i" of geographical
                // names in Hungarian:
                // Massachusetts ph:messzecsuzec
                // messzecsuzeci -> massachusettsi (adjective)
                // For lowercasing by conditional PFX rules, see
                // tests/germancompounding test example or the
                // Hungarian dictionary.)
                if (langnum == LANG_de || langnum == LANG_hu) {
                  std::string wordpart_lower(wordpart);
                  if (utf8) {
                    u8_u16(w, wordpart_lower);
                    mkallsmall_utf(w, langnum);
                    u16_u8(wordpart_lower, w);
                  } else {
                    mkallsmall(wordpart_lower, csconv);
                  }
                  reptable.push_back(replentry());
                  reptable.back().pattern.assign(ph);
                  reptable.back().outstrings[0].assign(wordpart_lower);
                }
                reptable.push_back(replentry());
                reptable.back().pattern.assign(ph_capitalized);
                reptable.back().outstrings[0].assign(wordpart);
              }
            }
            reptable.push_back(replentry());
            reptable.back().pattern.assign(ph);
            reptable.back().outstrings[0].assign(wordpart);
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