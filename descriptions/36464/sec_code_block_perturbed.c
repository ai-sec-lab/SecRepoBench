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