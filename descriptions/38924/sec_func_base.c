static struct mrb_insn_data
mrb_last_insn(codegen_scope *s)
{
  if (s->pc == 0) {
    struct mrb_insn_data data = { OP_NOP, 0 };
    return data;
  }
  return mrb_decode_insn(&s->iseq[s->lastpc]);
}