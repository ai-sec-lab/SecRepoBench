scpd = 0;
      wordnum = previouswordnum;
      numsyllable = oldnumsyllable;

      if (soldi != 0) {
        i = soldi;
        st.assign(word);  // XXX add more optim.
        soldi = 0;
      } else
        st[i] = ch;