static void
gen_move(codegen_scope *s, uint16_t dst, uint16_t src, int nopeep)
{
  if (no_peephole(s)) {
  normal:
    genop_2(s, OP_MOVE, dst, src);
    return;
  }
  else {
    struct mrb_insn_data data = mrb_last_insn(s);

    switch (data.insn) {
    case OP_MOVE:
      if (dst == src) return;             /* remove useless MOVE */
      if (data.b == dst && data.a == src) /* skip swapping MOVE */
        return;
      goto normal;
    case OP_LOADNIL: case OP_LOADSELF: case OP_LOADT: case OP_LOADF:
    case OP_LOADI__1:
    case OP_LOADI_0: case OP_LOADI_1: case OP_LOADI_2: case OP_LOADI_3:
    case OP_LOADI_4: case OP_LOADI_5: case OP_LOADI_6: case OP_LOADI_7:
      if (nopeep || data.a != src || data.a < s->nlocals) goto normal;
      s->pc = s->lastpc;
      genop_1(s, data.insn, dst);
      break;
    case OP_LOADI: case OP_LOADINEG: case OP_LOADI16:
    case OP_LOADL: case OP_LOADSYM:
    case OP_GETGV: case OP_GETSV: case OP_GETIV: case OP_GETCV:
    case OP_GETCONST: case OP_STRING:
    case OP_LAMBDA: case OP_BLOCK: case OP_METHOD: case OP_BLKPUSH:
      if (nopeep || data.a != src || data.a < s->nlocals) goto normal;
      s->pc = s->lastpc;
      genop_2(s, data.insn, dst, data.b);
      break;
    // <MASK>
    }
  }
}