static void
gen_assignment(codegen_scope *s, node *tree, node *rhs, int sp, int val)
{
  int idx;
  int type = nint(tree->car);

  switch (type) {
  case NODE_GVAR:
  case NODE_ARG:
  case NODE_LVAR:
  case NODE_IVAR:
  case NODE_CVAR:
  case NODE_CONST:
  case NODE_NIL:
  case NODE_MASGN:
    if (rhs) {
      codegen(s, rhs, VAL);
      pop();
      sp = cursp();
    }
    break;

  case NODE_COLON2:
  case NODE_CALL:
  case NODE_SCALL:
    /* keep evaluation order */
    break;

  case NODE_NVAR:
    codegen_error(s, "Can't assign to numbered parameter");
    break;

  default:
    codegen_error(s, "unknown lhs");
    break;
  }

  tree = tree->cdr;
  switch (type) {
  case NODE_GVAR:
    gen_setxv(s, OP_SETGV, sp, nsym(tree), val);
    break;
  case NODE_ARG:
  case NODE_LVAR:
    idx = lv_idx(s, nsym(tree));
    if (idx > 0) {
      if (idx != sp) {
        gen_move(s, idx, sp, val);
      }
      break;
    }
    else {                      /* upvar */
      gen_setupvar(s, sp, nsym(tree));
    }
    break;
  case NODE_IVAR:
    gen_setxv(s, OP_SETIV, sp, nsym(tree), val);
    break;
  case NODE_CVAR:
    gen_setxv(s, OP_SETCV, sp, nsym(tree), val);
    break;
  case NODE_CONST:
    gen_setxv(s, OP_SETCONST, sp, nsym(tree), val);
    break;
  case NODE_COLON2:
    if (sp) {
      gen_move(s, cursp(), sp, 0);
    }
    sp = cursp();
    push();
    codegen(s, tree->car, VAL);
    if (rhs) {
      codegen(s, rhs, VAL); pop();
      gen_move(s, sp, cursp(), 0);
    }
    pop_n(2);
    idx = new_sym(s, nsym(tree->cdr));
    genop_2(s, OP_SETMCNST, sp, idx);
    break;

  case NODE_CALL:
  case NODE_SCALL:
    {
      int noself = 0, safe = (type == NODE_SCALL), skip = 0, top, call, n = 0;
      mrb_sym mid = nsym(tree->cdr->car);

      top = cursp();
      if (val || sp == cursp()) {
        push();                   /* room for retval */
      }
      call = cursp();
      if (!tree->car) {
        noself = 1;
        push();
      }
      else {
        codegen(s, tree->car, VAL); /* receiver */
      }
      if (safe) {
        int recv = cursp()-1;
        gen_move(s, cursp(), recv, 1);
        skip = genjmp2_0(s, OP_JMPNIL, cursp(), val);
      }
      tree = tree->cdr->cdr->car;
      if (tree) {
        if (tree->car) {            /* positional arguments */
          n = gen_values(s, tree->car, VAL, (tree->cdr->car)?13:14);
          if (n < 0) {              /* variable length */
            n = 15;
            push();
          }
        }
        if (tree->cdr->car) {       /* keyword arguments */
          if (n == 13 || n == 14) {
            pop_n(n);
            genop_2(s, OP_ARRAY, cursp(), n);
            push();
            n = 15;
          }
          gen_hash(s, tree->cdr->car->cdr, VAL, 0);
          if (n < 14) {
            n++;
          }
          else {
            pop_n(2);
            genop_2(s, OP_ARYPUSH, cursp(), 1);
          }
          push();
        }
      }
      if (rhs) {
        codegen(s, rhs, VAL);
        pop();
      }
      else {
        gen_move(s, cursp(), sp, 0);
      }
      if (val) {
        gen_move(s, top, cursp(), 1);
      }
      // <MASK>
      if (safe) {
        dispatch(s, skip);
      }
      s->sp = top;
    }
    break;

  case NODE_MASGN:
    gen_massignment(s, tree->car, sp, val);
    break;

  /* splat without assignment */
  case NODE_NIL:
    break;

  default:
    codegen_error(s, "unknown lhs");
    break;
  }
  if (val) push();
}