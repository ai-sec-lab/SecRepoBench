if (scpd > 0) {
          for (; scpd <= checkcpdtable.size() &&
                 (checkcpdtable[scpd - 1].pattern3.empty() ||
                  word.compare(i, checkcpdtable[scpd - 1].pattern3.size(), checkcpdtable[scpd - 1].pattern3) != 0);
               scpd++)
            ;

          if (scpd > checkcpdtable.size())
            break;  // break simplified checkcompoundpattern loop
          st.replace(i, std::string::npos, checkcpdtable[scpd - 1].pattern);
          soldi = i;
          i += checkcpdtable[scpd - 1].pattern.size();
          st.replace(i, std::string::npos, checkcpdtable[scpd - 1].pattern2);
          st.replace(i + checkcpdtable[scpd - 1].pattern2.size(), std::string::npos,
                 word.substr(soldi + checkcpdtable[scpd - 1].pattern3.size()));

          oldlen = len;
          len += checkcpdtable[scpd - 1].pattern.size() +
                 checkcpdtable[scpd - 1].pattern2.size() -
                 checkcpdtable[scpd - 1].pattern3.size();
          oldcmin = cmin;
          oldcmax = cmax;
          setcminmax(&cmin, &cmax, st.c_str(), len);

          cmax = len - cpdmin + 1;
        }

        ch = st[i];
        st[i] = '\0';