if (scope->pc == 0) {
    struct mrb_insn_data data = { OP_NOP, 0 };
    return data;
  }
  return mrb_decode_insn(&scope->iseq[scope->lastpc]);