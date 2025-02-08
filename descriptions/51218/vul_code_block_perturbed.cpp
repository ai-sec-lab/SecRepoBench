if (ph.at(ph.size()-1) == '*') {
              strippatt = 1;
              size_t striplen = 0;
              if (utf8) {
                while ((strippatt < ph.size()) &&
                  ((ph.at(ph.size()-strippatt-1) & 0xc0) == 0x80))
                     ++strippatt;
                while ((striplen < wordpart.size()) &&
                  ((wordpart.at(wordpart.size()-striplen-1) & 0xc0) == 0x80))
                     ++striplen;
              }
              ++strippatt;
              ++striplen;
              if ((ph.size() > strippatt) && (wordpart.size() > striplen)) {
                ph.erase(ph.size()-strippatt, strippatt);
                wordpart.erase(in_word.size()-striplen, striplen);
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