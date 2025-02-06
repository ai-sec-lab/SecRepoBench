// check simplifiedtriple
            if (simplifiedtriple) {
              if (striple) {
                checkedstriple = 1;
                i--;  // check "fahrt" instead of "ahrt" in "Schiffahrt"
              } else if (i > 2 && word[i - 1] == word[i - 2])
                striple = 1;
            }