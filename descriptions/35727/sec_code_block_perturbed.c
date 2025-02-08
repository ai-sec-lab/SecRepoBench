if (a > 0xff) {
    gen_B(s, OP_EXT1);
    gen_B(s, instruction);
    gen_S(s, a);
  }
  else {
    gen_B(s, instruction);
    gen_B(s, (uint8_t)a);
  }
  pos = s->pc;
  gen_jmpdst(s, pc);