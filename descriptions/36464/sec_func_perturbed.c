static void
gen_move(codegen_scope *s, uint16_t destination, uint16_t src, int nopeep)
{
  if (no_peephole(s)) {
  normal:
    genop_2(s, OP_MOVE, destination, src);
    return;
  }
  else {
    struct mrb_insn_data data = mrb_last_insn(s);

    switch (data.insn) {
    case OP_MOVE:
      if (destination == src) return;             /* remove useless MOVE */
      if (data.b == destination && data.a == src) /* skip swapping MOVE */
        return;
      goto normal;
    case OP_LOADNIL: case OP_LOADSELF: case OP_LOADT: case OP_LOADF:
    case OP_LOADI__1:
    case OP_LOADI_0: case OP_LOADI_1: case OP_LOADI_2: case OP_LOADI_3:
    case OP_LOADI_4: case OP_LOADI_5: case OP_LOADI_6: case OP_LOADI_7:
      if (nopeep || data.a != src || data.a < s->nlocals) goto normal;
      s->pc = s->lastpc;
      genop_1(s, data.insn, destination);
      break;
    case OP_LOADI: case OP_LOADINEG: case OP_LOADI16:
    case OP_LOADL: case OP_LOADSYM:
    case OP_GETGV: case OP_GETSV: case OP_GETIV: case OP_GETCV:
    case OP_GETCONST: case OP_STRING:
    case OP_LAMBDA: case OP_BLOCK: case OP_METHOD: case OP_BLKPUSH:
      if (nopeep || data.a != src || data.a < s->nlocals) goto normal;
      s->pc = s->lastpc;
      genop_2(s, data.insn, destination, data.b);
      break;
    case OP_GETUPVAR:
      if (nopeep || data.a != src || data.a < s->nlocals) goto normal;
      s->pc = s->lastpc;
      genop_3(s, data.insn, destination, data.b, data.c);
      break;
    case OP_LOADI32:
      if (nopeep || data.a != src || data.a < s->nlocals) goto normal;
      else {
        uint32_t i = (uint32_t)data.b<<16|data.c;
        s->pc = s->lastpc;
        genop_2SS(s, data.insn, destination, i);
      }
      break;
    default:
      goto normal;
    }
  }
}