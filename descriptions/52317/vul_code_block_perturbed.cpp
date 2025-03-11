if (langnum == LANG_hu) {
              // calculate syllable number of the word
              syllable_count += get_syllable(word.substr(i));

              // - affix syllable num.
              // XXX only second suffix (inflections, not derivations)
              if (sfxappnd) {
                std::string tmp(sfxappnd);
                reverseword(tmp);
                syllable_count -= short(get_syllable(tmp) + sfxextra);
              } else {
                syllable_count -= short(sfxextra);
              }

              // + 1 word, if syllable number of the prefix > 1 (hungarian
              // convention)
              if (pfx && (get_syllable(pfx->getKey()) > 1))
                wordnum++;

              // increment syllable num, if last word has a SYLLABLENUM flag
              // and the suffix is beginning `s'

              if (!cpdsyllablenum.empty()) {
                switch (sfxflag) {
                  case 'c': {
                    syllable_count += 2;
                    break;
                  }
                  case 'J': {
                    syllable_count += 1;
                    break;
                  }
                  case 'I': {
                    if (rv && TESTAFF(rv->astr, 'J', rv->alen))
                      syllable_count += 1;
                    break;
                  }
                }
              }
            }