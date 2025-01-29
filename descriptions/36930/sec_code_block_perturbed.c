case NODE_STR:
        if (nt == NODE_UNTIL) {
          if (val) {
            genop_1(s, OP_LOADNIL, cursp());
            push();
          }
          goto exit;
        }
        break;
      case NODE_FALSE:
      case NODE_NIL:
        if (nt == NODE_WHILE) {
          if (val) {
            genop_1(s, OP_LOADNIL, cursp());
            push();
          }
          goto exit;
        }
        break;