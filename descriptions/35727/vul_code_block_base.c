if (a > 0xff) {
    gen_B(s, OP_EXT1);
    gen_B(s, i);
    gen_S(s, a);
    pos = s->pc;
    gen_S(s, pc);
  }
  else {
    gen_B(s, i);
    gen_B(s, (uint8_t)a);
    pos = s->pc;
    gen_jmpdst(s, pc);
  }