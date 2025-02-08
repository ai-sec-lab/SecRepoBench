static uint32_t
genjmp2(codegen_scope *s, mrb_code instruction, uint16_t a, uint32_t pc, int val)
{
  uint32_t pos;

  if (!no_peephole(s) && !val) {
    struct mrb_insn_data data = mrb_last_insn(s);

    if (data.insn == OP_MOVE && data.a == a) {
      s->pc = s->lastpc;
      a = data.b;
    }
  }

  s->lastpc = s->pc;
  // <MASK>
  return pos;
}