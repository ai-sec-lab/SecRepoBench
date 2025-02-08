case OP_GETUPVAR: case OP_LOADI32:
      if (nopeep || data.a != src || data.a < s->nlocals) goto normal;
      s->pc = s->lastpc;
      genop_3(s, data.insn, dst, data.b, data.c);
      break;
    default:
      goto normal;