if (n < 15) {
        n++;
        if (n == 15) {
          pop_n(14);
          genop_2(s, OP_ARRAY, cursp(), 15);
        }
      }
      else {
        pop();
        genop_2(s, OP_ARYPUSH, cursp(), 1);
      }
      push(); pop();
      s->sp = call;
      if (mid == MRB_OPSYM_2(s->mrb, aref) && n == 2) {
        genop_1(s, OP_SETIDX, cursp());
      }
      else {
        genop_3(s, noself ? OP_SSEND : OP_SEND, cursp(), new_sym(s, attrsym(s, mid)), n);
      }