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
  // <MASK>
}