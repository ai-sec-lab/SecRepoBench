{
        i = soldi;
        st.assign(word);  // XXX add more optim.
        soldi = 0;
        len = oldlen;
        cmin = oldcmin;
        cmax = oldcmax;
      } else
        st[i] = ch;