MRB_API mrb_value
mrb_vm_exec(mrb_state *mrb, const struct RProc *proc, const mrb_code *pc)
{
  /* mrb_assert(MRB_PROC_CFUNC_P(proc)) */
  const mrb_irep *irep = proc->body.irep;
  const mrb_pool_value *pool = irep->pool;
  const mrb_sym *syms = irep->syms;
  mrb_code insn;
  int ai = mrb_gc_arena_save(mrb);
  struct mrb_jmpbuf *prev_jmp = mrb->jmp;
  struct mrb_jmpbuf c_jmp;
  uint32_t a;
  uint16_t b;
  uint16_t c;
  mrb_sym methodid;
  const struct mrb_irep_catch_handler *ch;

#ifdef DIRECT_THREADED
  static const void * const optable[] = {
#define OPCODE(x,_) &&L_OP_ ## x,
#include "mruby/ops.h"
#undef OPCODE
  };
#endif

  mrb_bool exc_catched = FALSE;
RETRY_TRY_BLOCK:

  MRB_TRY(&c_jmp) {

  if (exc_catched) {
    exc_catched = FALSE;
    mrb_gc_arena_restore(mrb, ai);
    if (mrb->exc && mrb->exc->tt == MRB_TT_BREAK)
      goto L_BREAK;
    goto L_RAISE;
  }
  mrb->jmp = &c_jmp;
  mrb_vm_ci_proc_set(mrb->c->ci, proc);

#define regs (mrb->c->ci->stack)
  INIT_DISPATCH {
    CASE(OP_NOP, Z) {
      /* do nothing */
      NEXT;
    }

    CASE(OP_MOVE, BB) {
      regs[a] = regs[b];
      NEXT;
    }

    CASE(OP_LOADL, BB) {
      switch (pool[b].tt) {   /* number */
      case IREP_TT_INT32:
        regs[a] = mrb_int_value(mrb, (mrb_int)pool[b].u.i32);
        break;
      case IREP_TT_INT64:
#if defined(MRB_INT64)
        regs[a] = mrb_int_value(mrb, (mrb_int)pool[b].u.i64);
        break;
#else
#if defined(MRB_64BIT)
        if (INT32_MIN <= pool[b].u.i64 && pool[b].u.i64 <= INT32_MAX) {
          regs[a] = mrb_int_value(mrb, (mrb_int)pool[b].u.i64);
          break;
        }
#endif
        goto L_INT_OVERFLOW;
#endif
      case IREP_TT_BIGINT:
        goto L_INT_OVERFLOW;
#ifndef MRB_NO_FLOAT
      case IREP_TT_FLOAT:
        regs[a] = mrb_float_value(mrb, pool[b].u.f);
        break;
#endif
      default:
        /* should not happen (tt:string) */
        regs[a] = mrb_nil_value();
        break;
      }
      NEXT;
    }

    CASE(OP_LOADI, BB) {
      SET_FIXNUM_VALUE(regs[a], b);
      NEXT;
    }

    CASE(OP_LOADINEG, BB) {
      SET_FIXNUM_VALUE(regs[a], -b);
      NEXT;
    }

    CASE(OP_LOADI__1,B) goto L_LOADI;
    CASE(OP_LOADI_0,B) goto L_LOADI;
    CASE(OP_LOADI_1,B) goto L_LOADI;
    CASE(OP_LOADI_2,B) goto L_LOADI;
    CASE(OP_LOADI_3,B) goto L_LOADI;
    CASE(OP_LOADI_4,B) goto L_LOADI;
    CASE(OP_LOADI_5,B) goto L_LOADI;
    CASE(OP_LOADI_6,B) goto L_LOADI;
    CASE(OP_LOADI_7, B) {
    L_LOADI:
      SET_FIXNUM_VALUE(regs[a], (mrb_int)insn - (mrb_int)OP_LOADI_0);
      NEXT;
    }

    CASE(OP_LOADI16, BS) {
      SET_FIXNUM_VALUE(regs[a], (mrb_int)(int16_t)b);
      NEXT;
    }

    CASE(OP_LOADI32, BSS) {
      SET_INT_VALUE(mrb, regs[a], (int32_t)(((uint32_t)b<<16)+c));
      NEXT;
    }

    CASE(OP_LOADSYM, BB) {
      SET_SYM_VALUE(regs[a], syms[b]);
      NEXT;
    }

    CASE(OP_LOADNIL, B) {
      SET_NIL_VALUE(regs[a]);
      NEXT;
    }

    CASE(OP_LOADSELF, B) {
      regs[a] = regs[0];
      NEXT;
    }

    CASE(OP_LOADT, B) {
      SET_TRUE_VALUE(regs[a]);
      NEXT;
    }

    CASE(OP_LOADF, B) {
      SET_FALSE_VALUE(regs[a]);
      NEXT;
    }

    CASE(OP_GETGV, BB) {
      mrb_value val = mrb_gv_get(mrb, syms[b]);
      regs[a] = val;
      NEXT;
    }

    CASE(OP_SETGV, BB) {
      mrb_gv_set(mrb, syms[b], regs[a]);
      NEXT;
    }

    CASE(OP_GETSV, BB) {
      mrb_value val = mrb_vm_special_get(mrb, syms[b]);
      regs[a] = val;
      NEXT;
    }

    CASE(OP_SETSV, BB) {
      mrb_vm_special_set(mrb, syms[b], regs[a]);
      NEXT;
    }

    CASE(OP_GETIV, BB) {
      regs[a] = mrb_iv_get(mrb, regs[0], syms[b]);
      NEXT;
    }

    CASE(OP_SETIV, BB) {
      mrb_iv_set(mrb, regs[0], syms[b], regs[a]);
      NEXT;
    }

    CASE(OP_GETCV, BB) {
      mrb_value val;
      val = mrb_vm_cv_get(mrb, syms[b]);
      regs[a] = val;
      NEXT;
    }

    CASE(OP_SETCV, BB) {
      mrb_vm_cv_set(mrb, syms[b], regs[a]);
      NEXT;
    }

    CASE(OP_GETCONST, BB) {
      mrb_value val;
      mrb_sym sym = syms[b];

      val = mrb_vm_const_get(mrb, sym);
      regs[a] = val;
      NEXT;
    }

    CASE(OP_SETCONST, BB) {
      mrb_vm_const_set(mrb, syms[b], regs[a]);
      NEXT;
    }

    CASE(OP_GETMCNST, BB) {
      mrb_value val;

      val = mrb_const_get(mrb, regs[a], syms[b]);
      regs[a] = val;
      NEXT;
    }

    CASE(OP_SETMCNST, BB) {
      mrb_const_set(mrb, regs[a+1], syms[b], regs[a]);
      NEXT;
    }

    CASE(OP_GETUPVAR, BBB) {
      mrb_value *regs_a = regs + a;
      struct REnv *e = uvenv(mrb, c);

      if (e && b < MRB_ENV_LEN(e)) {
        *regs_a = e->stack[b];
      }
      else {
        *regs_a = mrb_nil_value();
      }
      NEXT;
    }

    CASE(OP_SETUPVAR, BBB) {
      struct REnv *e = uvenv(mrb, c);

      if (e) {
        mrb_value *regs_a = regs + a;

        if (b < MRB_ENV_LEN(e)) {
          e->stack[b] = *regs_a;
          mrb_write_barrier(mrb, (struct RBasic*)e);
        }
      }
      NEXT;
    }

    CASE(OP_JMP, S) {
      pc += (int16_t)a;
      JUMP;
    }
    CASE(OP_JMPIF, BS) {
      if (mrb_test(regs[a])) {
        pc += (int16_t)b;
        JUMP;
      }
      NEXT;
    }
    CASE(OP_JMPNOT, BS) {
      if (!mrb_test(regs[a])) {
        pc += (int16_t)b;
        JUMP;
      }
      NEXT;
    }
    CASE(OP_JMPNIL, BS) {
      if (mrb_nil_p(regs[a])) {
        pc += (int16_t)b;
        JUMP;
      }
      NEXT;
    }

    CASE(OP_JMPUW, S) {
      a = (uint32_t)((pc - irep->iseq) + (int16_t)a);
      CHECKPOINT_RESTORE(RBREAK_TAG_JUMP) {
        struct RBreak *brk = (struct RBreak*)mrb->exc;
        mrb_value target = mrb_break_value_get(brk);
        mrb_assert(mrb_integer_p(target));
        a = (uint32_t)mrb_integer(target);
        mrb_assert(a >= 0 && a < irep->ilen);
      }
      CHECKPOINT_MAIN(RBREAK_TAG_JUMP) {
        ch = catch_handler_find(mrb, mrb->c->ci, pc, MRB_CATCH_FILTER_ENSURE);
        if (ch) {
          /* avoiding a jump from a catch handler into the same handler */
          if (a < mrb_irep_catch_handler_unpack(ch->begin) || a >= mrb_irep_catch_handler_unpack(ch->end)) {
            THROW_TAGGED_BREAK(mrb, RBREAK_TAG_JUMP, proc, mrb_fixnum_value(a));
          }
        }
      }
      CHECKPOINT_END(RBREAK_TAG_JUMP);

      mrb->exc = NULL; /* clear break object */
      pc = irep->iseq + a;
      JUMP;
    }

    CASE(OP_EXCEPT, B) {
      mrb_value exc;

      if (mrb->exc == NULL) {
        exc = mrb_nil_value();
      }
      else {
        switch (mrb->exc->tt) {
        case MRB_TT_BREAK:
        case MRB_TT_EXCEPTION:
          exc = mrb_obj_value(mrb->exc);
          break;
        default:
          mrb_assert(!"bad mrb_type");
          exc = mrb_nil_value();
          break;
        }
        mrb->exc = NULL;
      }
      regs[a] = exc;
      NEXT;
    }
    CASE(OP_RESCUE, BB) {
      mrb_value exc = regs[a];  /* exc on stack */
      mrb_value e = regs[b];
      struct RClass *ec;

      switch (mrb_type(e)) {
      case MRB_TT_CLASS:
      case MRB_TT_MODULE:
        break;
      default:
        {
          mrb_value exc;

          exc = mrb_exc_new_lit(mrb, E_TYPE_ERROR,
                                    "class or module required for rescue clause");
          mrb_exc_set(mrb, exc);
          goto L_RAISE;
        }
      }
      ec = mrb_class_ptr(e);
      regs[b] = mrb_bool_value(mrb_obj_is_kind_of(mrb, exc, ec));
      NEXT;
    }

    CASE(OP_RAISEIF, B) {
      mrb_value exc = regs[a];
      if (mrb_break_p(exc)) {
        mrb->exc = mrb_obj_ptr(exc);
        goto L_BREAK;
      }
      mrb_exc_set(mrb, exc);
      if (mrb->exc) {
        goto L_RAISE;
      }
      NEXT;
    }

    CASE(OP_SENDV, BB) {
      c = CALL_MAXARGS;
      goto L_SEND;
    };

    CASE(OP_SENDVB, BB) {
      c = CALL_MAXARGS;
      goto L_SENDB;
    };

    CASE(OP_SEND, BBB)
    L_SEND:
    {
      /* push nil after arguments */
      int bidx = (c == CALL_MAXARGS) ? a+2 : a+c+1;
      SET_NIL_VALUE(regs[bidx]);
      goto L_SENDB;
    };
    L_SEND_SYM:
    {
      /* push nil after arguments */
      int bidx = (c == CALL_MAXARGS) ? a+2 : a+c+1;
      SET_NIL_VALUE(regs[bidx]);
      goto L_SENDB_SYM;
    };

    CASE(OP_SENDB, BBB)
    L_SENDB:
    methodid = syms[b];
    L_SENDB_SYM:
    {
      mrb_int argc = (c == CALL_MAXARGS) ? -1 : c;
      mrb_int bidx = (argc < 0) ? a+2 : a+c+1;
      mrb_method_t m;
      struct RClass *cls;
      mrb_callinfo *ci = mrb->c->ci;
      mrb_value recv, blk;

      mrb_assert(bidx < irep->nregs);

      recv = regs[a];
      blk = regs[bidx];
      if (!mrb_nil_p(blk) && !mrb_proc_p(blk)) {
        blk = mrb_type_convert(mrb, blk, MRB_TT_PROC, MRB_SYM(to_proc));
        /* The stack might have been reallocated during mrb_type_convert(),
           see #3622 */
        regs[bidx] = blk;
      }
      cls = mrb_class(mrb, recv);
      m = mrb_method_search_vm(mrb, &cls, methodid);
      if (MRB_METHOD_UNDEF_P(m)) {
        mrb_sym missing = MRB_SYM(method_missing);
        mrb_value args;

        if (mrb_func_basic_p(mrb, recv, missing, mrb_obj_missing)) {
        method_missing:
          args = get_send_args(mrb, argc, regs+a+1);
          mrb_method_missing(mrb, methodid, recv, args);
        }
        if (methodid != missing) {
          cls = mrb_class(mrb, recv);
        }
        m = mrb_method_search_vm(mrb, &cls, missing);
        if (MRB_METHOD_UNDEF_P(m)) goto method_missing; /* just in case */
        if (argc >= 0) {
          if (a+2 >= irep->nregs) {
            mrb_stack_extend(mrb, a+3);
          }
          regs[a+1] = mrb_ary_new_from_values(mrb, c, regs+a+1);
          regs[a+2] = blk;
          argc = -1;
        }
        mrb_ary_unshift(mrb, regs[a+1], mrb_symbol_value(methodid));
        methodid = missing;
      }

      /* push callinfo */
      ci = cipush(mrb, a, a, cls, NULL, methodid, argc);

      if (MRB_METHOD_CFUNC_P(m)) {
        if (MRB_METHOD_PROC_P(m)) {
          struct RProc *p = MRB_METHOD_PROC(m);

          mrb_vm_ci_proc_set(ci, p);
          recv = p->body.func(mrb, recv);
        }
        else if (MRB_METHOD_NOARG_P(m) &&
                 (argc > 0 || (argc == -1 && RARRAY_LEN(regs[1]) != 0))) {
          argnum_error(mrb, 0);
          goto L_RAISE;
        }
        else {
          recv = MRB_METHOD_FUNC(m)(mrb, recv);
        }
        mrb_gc_arena_restore(mrb, ai);
        mrb_gc_arena_shrink(mrb, ai);
        if (mrb->exc) goto L_RAISE;
        ci = mrb->c->ci;
        if (mrb_proc_p(blk)) {
          struct RProc *p = mrb_proc_ptr(blk);
          if (p && !MRB_PROC_STRICT_P(p) && MRB_PROC_ENV(p) == mrb_vm_ci_env(&ci[-1])) {
            p->flags |= MRB_PROC_ORPHAN;
          }
        }
        if (!ci->u.target_class) { /* return from context modifying method (resume/yield) */
          if (ci->acc == CI_ACC_RESUMED) {
            mrb->jmp = prev_jmp;
            return recv;
          }
          else {
            mrb_assert(!MRB_PROC_CFUNC_P(ci[-1].proc));
            proc = ci[-1].proc;
            irep = proc->body.irep;
            pool = irep->pool;
            syms = irep->syms;
          }
        }
        mrb->c->ci->stack[0] = recv;
        /* pop stackpos */
        ci = cipop(mrb);
        pc = ci->pc;
      }
      else {
        /* setup environment for calling method */
        mrb_vm_ci_proc_set(ci, (proc = MRB_METHOD_PROC(m)));
        irep = proc->body.irep;
        pool = irep->pool;
        syms = irep->syms;
        mrb_stack_extend(mrb, (argc < 0 && irep->nregs < 3) ? 3 : irep->nregs);
        pc = irep->iseq;
      }
    }
    JUMP;

    CASE(OP_CALL, Z) {
      mrb_callinfo *ci;
      mrb_value recv = mrb->c->ci->stack[0];
      struct RProc *m = mrb_proc_ptr(recv);

      /* replace callinfo */
      ci = mrb->c->ci;
      ci->u.target_class = MRB_PROC_TARGET_CLASS(m);
      mrb_vm_ci_proc_set(ci, m);
      if (MRB_PROC_ENV_P(m)) {
        ci->mid = MRB_PROC_ENV(m)->mid;
      }

      /* prepare stack */
      if (MRB_PROC_CFUNC_P(m)) {
        recv = MRB_PROC_CFUNC(m)(mrb, recv);
        mrb_gc_arena_restore(mrb, ai);
        mrb_gc_arena_shrink(mrb, ai);
        if (mrb->exc) goto L_RAISE;
        /* pop stackpos */
        ci = cipop(mrb);
        pc = ci->pc;
        regs[ci[1].acc] = recv;
        irep = mrb->c->ci->proc->body.irep;
        pool = irep->pool;
        syms = irep->syms;
        JUMP;
      }
      else {
        /* setup environment for calling method */
        proc = m;
        irep = m->body.irep;
        if (!irep) {
          mrb->c->ci->stack[0] = mrb_nil_value();
          a = 0;
          c = OP_R_NORMAL;
          goto L_OP_RETURN_BODY;
        }
        pool = irep->pool;
        syms = irep->syms;
        mrb_stack_extend(mrb, irep->nregs);
        if (ci->argc < 0) {
          if (irep->nregs > 3) {
            stack_clear(regs+3, irep->nregs-3);
          }
        }
        else if (ci->argc+2 < irep->nregs) {
          stack_clear(regs+ci->argc+2, irep->nregs-ci->argc-2);
        }
        if (MRB_PROC_ENV_P(m)) {
          regs[0] = MRB_PROC_ENV(m)->stack[0];
        }
        pc = irep->iseq;
        JUMP;
      }
    }

    CASE(OP_SUPER, BB) {
      mrb_int argc = (b == CALL_MAXARGS) ? -1 : b;
      int bidx = (argc < 0) ? a+2 : a+b+1;
      mrb_method_t m;
      struct RClass *cls;
      mrb_callinfo *ci = mrb->c->ci;
      mrb_value recv, blk;
      const struct RProc *p = ci->proc;
      mrb_sym methodid = ci->mid;
      struct RClass* target_class = MRB_PROC_TARGET_CLASS(p);

      if (MRB_PROC_ENV_P(p) && p->e.env->mid && p->e.env->mid != methodid) { /* alias support */
        methodid = p->e.env->mid;    /* restore old mid */
      }
      mrb_assert(bidx < irep->nregs);

      if (methodid == 0 || !target_class) {
        mrb_value exc = mrb_exc_new_lit(mrb, E_NOMETHOD_ERROR, "super called outside of method");
        mrb_exc_set(mrb, exc);
        goto L_RAISE;
      }
      if (target_class->flags & MRB_FL_CLASS_IS_PREPENDED) {
        target_class = mrb_vm_ci_target_class(ci);
      }
      else if (target_class->tt == MRB_TT_MODULE) {
        target_class = mrb_vm_ci_target_class(ci);
        if (target_class->tt != MRB_TT_ICLASS) {
          goto super_typeerror;
        }
      }
      recv = regs[0];
      if (!mrb_obj_is_kind_of(mrb, recv, target_class)) {
      super_typeerror: ;
        mrb_value exc = mrb_exc_new_lit(mrb, E_TYPE_ERROR,
                                            "self has wrong type to call super in this context");
        mrb_exc_set(mrb, exc);
        goto L_RAISE;
      }
      blk = regs[bidx];
      if (!mrb_nil_p(blk) && !mrb_proc_p(blk)) {
        blk = mrb_type_convert(mrb, blk, MRB_TT_PROC, MRB_SYM(to_proc));
        /* The stack or ci stack might have been reallocated during
           mrb_type_convert(), see #3622 and #3784 */
        regs[bidx] = blk;
        ci = mrb->c->ci;
      }
      cls = target_class->super;
      m = mrb_method_search_vm(mrb, &cls, methodid);
      if (MRB_METHOD_UNDEF_P(m)) {
        mrb_sym missing = MRB_SYM(method_missing);
        mrb_value args;

        if (mrb_func_basic_p(mrb, recv, missing, mrb_obj_missing)) {
        super_missing:
          args = get_send_args(mrb, argc, regs+a+1);
          mrb_no_method_error(mrb, methodid, args, "no superclass method '%n'", methodid);
        }
        if (methodid != missing) {
          cls = mrb_class(mrb, recv);
        }
        m = mrb_method_search_vm(mrb, &cls, missing);
        if (MRB_METHOD_UNDEF_P(m)) goto super_missing; /* just in case */
        if (argc >= 0) {
          if (a+2 >= irep->nregs) {
            mrb_stack_extend(mrb, a+3);
          }
          regs[a+1] = mrb_ary_new_from_values(mrb, b, regs+a+1);
          regs[a+2] = blk;
          argc = -1;
        }
        mrb_ary_unshift(mrb, regs[a+1], mrb_symbol_value(methodid));
        methodid = missing;
      }

      /* push callinfo */
      ci = cipush(mrb, a, 0, cls, NULL, methodid, argc);

      /* prepare stack */
      mrb->c->ci->stack[0] = recv;

      if (MRB_METHOD_CFUNC_P(m)) {
        mrb_value v;

        if (MRB_METHOD_PROC_P(m)) {
          mrb_vm_ci_proc_set(ci, MRB_METHOD_PROC(m));
        }
        v = MRB_METHOD_CFUNC(m)(mrb, recv);
        mrb_gc_arena_restore(mrb, ai);
        if (mrb->exc) goto L_RAISE;
        ci = mrb->c->ci;
        mrb_assert(!mrb_break_p(v));
        if (!mrb_vm_ci_target_class(ci)) { /* return from context modifying method (resume/yield) */
          if (ci->acc == CI_ACC_RESUMED) {
            mrb->jmp = prev_jmp;
            return v;
          }
          else {
            mrb_assert(!MRB_PROC_CFUNC_P(ci[-1].proc));
            proc = ci[-1].proc;
            irep = proc->body.irep;
            pool = irep->pool;
            syms = irep->syms;
          }
        }
        mrb->c->ci->stack[0] = v;
        ci = cipop(mrb);
        pc = ci->pc;
        JUMP;
      }
      else {
        /* fill callinfo */
        ci->acc = a;

        /* setup environment for calling method */
        mrb_vm_ci_proc_set(ci, (proc = MRB_METHOD_PROC(m)));
        irep = proc->body.irep;
        pool = irep->pool;
        syms = irep->syms;
        mrb_stack_extend(mrb, (argc < 0 && irep->nregs < 3) ? 3 : irep->nregs);
        pc = irep->iseq;
        JUMP;
      }
    }

    CASE(OP_ARGARY, BS) {
      mrb_int m1 = (b>>11)&0x3f;
      mrb_int r  = (b>>10)&0x1;
      mrb_int m2 = (b>>5)&0x1f;
      mrb_int kd = (b>>4)&0x1;
      mrb_int lv = (b>>0)&0xf;
      mrb_value *stack;

      if (mrb->c->ci->mid == 0 || mrb_vm_ci_target_class(mrb->c->ci) == NULL) {
        mrb_value exc;

      L_NOSUPER:
        exc = mrb_exc_new_lit(mrb, E_NOMETHOD_ERROR, "super called outside of method");
        mrb_exc_set(mrb, exc);
        goto L_RAISE;
      }
      if (lv == 0) stack = regs + 1;
      else {
        struct REnv *e = uvenv(mrb, lv-1);
        if (!e) goto L_NOSUPER;
        if (MRB_ENV_LEN(e) <= m1+r+m2+kd+1)
          goto L_NOSUPER;
        stack = e->stack + 1;
      }
      if (r == 0) {
        regs[a] = mrb_ary_new_from_values(mrb, m1+m2+kd, stack);
      }
      else {
        mrb_value *pp = NULL;
        struct RArray *rest;
        mrb_int len = 0;

        if (mrb_array_p(stack[m1])) {
          struct RArray *ary = mrb_ary_ptr(stack[m1]);

          pp = ARY_PTR(ary);
          len = ARY_LEN(ary);
        }
        regs[a] = mrb_ary_new_capa(mrb, m1+len+m2+kd);
        rest = mrb_ary_ptr(regs[a]);
        if (m1 > 0) {
          stack_copy(ARY_PTR(rest), stack, m1);
        }
        if (len > 0) {
          stack_copy(ARY_PTR(rest)+m1, pp, len);
        }
        if (m2 > 0) {
          stack_copy(ARY_PTR(rest)+m1+len, stack+m1+1, m2);
        }
        if (kd) {
          stack_copy(ARY_PTR(rest)+m1+len+m2, stack+m1+m2+1, kd);
        }
        ARY_SET_LEN(rest, m1+len+m2+kd);
      }
      regs[a+1] = stack[m1+r+m2];
      mrb_gc_arena_restore(mrb, ai);
      NEXT;
    }

    CASE(OP_ENTER, W) {
      mrb_int m1 = MRB_ASPEC_REQ(a);
      mrb_int o  = MRB_ASPEC_OPT(a);
      mrb_int r  = MRB_ASPEC_REST(a);
      mrb_int m2 = MRB_ASPEC_POST(a);
      mrb_int kd = (MRB_ASPEC_KEY(a) > 0 || MRB_ASPEC_KDICT(a))? 1 : 0;
      /* unused
      int b  = MRB_ASPEC_BLOCK(a);
      */
      mrb_int argc = mrb->c->ci->argc;
      mrb_value *argv = regs+1;
      mrb_value * const argv0 = argv;
      mrb_int const len = m1 + o + r + m2;
      mrb_int const blk_pos = len + kd + 1;
      mrb_value *blk = &argv[argc < 0 ? 1 : argc];
      mrb_value kdict = mrb_nil_value();
      mrb_int kargs = kd;

      /* arguments is passed with Array */
      if (argc < 0) {
        struct RArray *ary = mrb_ary_ptr(regs[1]);
        argv = ARY_PTR(ary);
        argc = (int)ARY_LEN(ary);
        mrb_gc_protect(mrb, regs[1]);
      }

      /* strict argument check */
      if (mrb->c->ci->proc && MRB_PROC_STRICT_P(mrb->c->ci->proc)) {
        if (argc < m1 + m2 || (r == 0 && argc > len + kd)) {
          argnum_error(mrb, m1+m2);
          goto L_RAISE;
        }
      }
      /* extract first argument array to arguments */
      else if (len > 1 && argc == 1 && mrb_array_p(argv[0])) {
        mrb_gc_protect(mrb, argv[0]);
        argc = (int)RARRAY_LEN(argv[0]);
        argv = RARRAY_PTR(argv[0]);
      }

      if (kd) {
        /* check last arguments is hash if method takes keyword arguments */
        if (argc == m1+m2) {
          kdict = mrb_hash_new(mrb);
          kargs = 0;
        }
        else {
          if (argv && argc > 0 && mrb_hash_p(argv[argc-1])) {
            kdict = argv[argc-1];
            mrb_hash_check_kdict(mrb, kdict);
          }
          else if (r || argc <= m1+m2+o
                   || !(mrb->c->ci->proc && MRB_PROC_STRICT_P(mrb->c->ci->proc))) {
            kdict = mrb_hash_new(mrb);
            kargs = 0;
          }
          else {
            argnum_error(mrb, m1+m2);
            goto L_RAISE;
          }
          if (MRB_ASPEC_KEY(a) > 0) {
            kdict = mrb_hash_dup(mrb, kdict);
          }
        }
      }

      /* no rest arguments */
      if (argc-kargs < len) {
        mrb_int mlen = m2;
        if (argc < m1+m2) {
          mlen = m1 < argc ? argc - m1 : 0;
        }
        regs[blk_pos] = *blk; /* move block */
        if (kd) regs[len + 1] = kdict;

        /* copy mandatory and optional arguments */
        if (argv0 != argv && argv) {
          value_move(&regs[1], argv, argc-mlen); /* m1 + o */
        }
        if (argc < m1) {
          stack_clear(&regs[argc+1], m1-argc);
        }
        /* copy post mandatory arguments */
        if (mlen) {
          value_move(&regs[len-m2+1], &argv[argc-mlen], mlen);
        }
        if (mlen < m2) {
          stack_clear(&regs[len-m2+mlen+1], m2-mlen);
        }
        /* initialize rest arguments with empty Array */
        if (r) {
          regs[m1+o+1] = mrb_ary_new_capa(mrb, 0);
        }
        /* skip initializer of passed arguments */
        if (o > 0 && argc-kargs > m1+m2)
          pc += (argc - kargs - m1 - m2)*3;
      }
      else {
        mrb_int rnum = 0;
        if (argv0 != argv) {
          regs[blk_pos] = *blk; /* move block */
          if (kd) regs[len + 1] = kdict;
          value_move(&regs[1], argv, m1+o);
        }
        if (r) {
          mrb_value ary;

          rnum = argc-m1-o-m2-kargs;
          ary = mrb_ary_new_from_values(mrb, rnum, argv+m1+o);
          regs[m1+o+1] = ary;
        }
        if (m2) {
          if (argc-m2 > m1) {
            value_move(&regs[m1+o+r+1], &argv[m1+o+rnum], m2);
          }
        }
        if (argv0 == argv) {
          regs[blk_pos] = *blk; /* move block */
          if (kd) regs[len + 1] = kdict;
        }
        pc += o*3;
      }

      /* format arguments for generated code */
      mrb->c->ci->argc = (int16_t)(len + kd);

      /* clear local (but non-argument) variables */
      if (irep->nlocals-blk_pos-1 > 0) {
        stack_clear(&regs[blk_pos+1], irep->nlocals-blk_pos-1);
      }
      JUMP;
    }

    CASE(OP_KARG, BB) {
      mrb_value k = mrb_symbol_value(syms[b]);
      mrb_value kdict = regs[mrb->c->ci->argc];

      if (!mrb_hash_p(kdict) || !mrb_hash_key_p(mrb, kdict, k)) {
        mrb_value str = mrb_format(mrb, "missing keyword: %v", k);
        mrb_exc_set(mrb, mrb_exc_new_str(mrb, E_ARGUMENT_ERROR, str));
        goto L_RAISE;
      }
      regs[a] = mrb_hash_get(mrb, kdict, k);
      mrb_hash_delete_key(mrb, kdict, k);
      NEXT;
    }

    CASE(OP_KEY_P, BB) {
      mrb_value k = mrb_symbol_value(syms[b]);
      mrb_value kdict = regs[mrb->c->ci->argc];
      mrb_bool key_p = FALSE;

      if (mrb_hash_p(kdict)) {
        key_p = mrb_hash_key_p(mrb, kdict, k);
      }
      regs[a] = mrb_bool_value(key_p);
      NEXT;
    }

    CASE(OP_KEYEND, Z) {
      mrb_value kdict = regs[mrb->c->ci->argc];

      if (mrb_hash_p(kdict) && !mrb_hash_empty_p(mrb, kdict)) {
        mrb_value keys = mrb_hash_keys(mrb, kdict);
        mrb_value key1 = RARRAY_PTR(keys)[0];
        mrb_value str = mrb_format(mrb, "unknown keyword: %v", key1);
        mrb_exc_set(mrb, mrb_exc_new_str(mrb, E_ARGUMENT_ERROR, str));
        goto L_RAISE;
      }
      NEXT;
    }

    CASE(OP_BREAK, B) {
      c = OP_R_BREAK;
      goto L_RETURN;
    }
    CASE(OP_RETURN_BLK, B) {
      c = OP_R_RETURN;
      goto L_RETURN;
    }
    CASE(OP_RETURN, B)
    c = OP_R_NORMAL;
    L_RETURN:
    {
      mrb_callinfo *ci;

      ci = mrb->c->ci;
      if (ci->mid) {
        mrb_value blk;

        if (ci->argc < 0) {
          blk = regs[2];
        }
        else {
          blk = regs[ci->argc+1];
        }
        if (mrb_proc_p(blk)) {
          struct RProc *p = mrb_proc_ptr(blk);

          if (!MRB_PROC_STRICT_P(p) &&
              ci > mrb->c->cibase && MRB_PROC_ENV(p) == mrb_vm_ci_env(&ci[-1])) {
            p->flags |= MRB_PROC_ORPHAN;
          }
        }
      }

      if (mrb->exc) {
      L_RAISE:
        ci = mrb->c->ci;
        if (ci == mrb->c->cibase) {
          ch = catch_handler_find(mrb, ci, pc, MRB_CATCH_FILTER_ALL);
          if (ch == NULL) goto L_FTOP;
          goto L_CATCH;
        }
        while ((ch = catch_handler_find(mrb, ci, pc, MRB_CATCH_FILTER_ALL)) == NULL) {
          ci = cipop(mrb);
          if (ci[1].acc == CI_ACC_SKIP && prev_jmp) {
            mrb->jmp = prev_jmp;
            MRB_THROW(prev_jmp);
          }
          pc = ci[0].pc;
          if (ci == mrb->c->cibase) {
            ch = catch_handler_find(mrb, ci, pc, MRB_CATCH_FILTER_ALL);
            if (ch == NULL) {
            L_FTOP:             /* fiber top */
              if (mrb->c == mrb->root_c) {
                mrb->c->ci->stack = mrb->c->stbase;
                goto L_STOP;
              }
              else {
                struct mrb_context *c = mrb->c;

                c->status = MRB_FIBER_TERMINATED;
                mrb->c = c->prev;
                c->prev = NULL;
                goto L_RAISE;
              }
            }
            break;
          }
        }
      L_CATCH:
        if (ch == NULL) goto L_STOP;
        if (FALSE) {
        L_CATCH_TAGGED_BREAK: /* from THROW_TAGGED_BREAK() or UNWIND_ENSURE() */
          ci = mrb->c->ci;
        }
        proc = ci->proc;
        irep = proc->body.irep;
        pool = irep->pool;
        syms = irep->syms;
        mrb_stack_extend(mrb, irep->nregs);
        pc = irep->iseq + mrb_irep_catch_handler_unpack(ch->target);
      }
      else {
        mrb_int acc;
        mrb_value v;

        ci = mrb->c->ci;
        v = regs[a];
        mrb_gc_protect(mrb, v);
        switch (c) {
        case OP_R_RETURN:
          /* Fall through to OP_R_NORMAL otherwise */
          if (ci->acc >=0 && MRB_PROC_ENV_P(proc) && !MRB_PROC_STRICT_P(proc)) {
            const struct RProc *dst;
            mrb_callinfo *cibase;
            cibase = mrb->c->cibase;
            dst = top_proc(mrb, proc);

            if (MRB_PROC_ENV_P(dst)) {
              struct REnv *e = MRB_PROC_ENV(dst);

              if (!MRB_ENV_ONSTACK_P(e) || (e->cxt && e->cxt != mrb->c)) {
                localjump_error(mrb, LOCALJUMP_ERROR_RETURN);
                goto L_RAISE;
              }
            }
            /* check jump destination */
            while (cibase <= ci && ci->proc != dst) {
              if (ci->acc < 0) { /* jump cross C boundary */
                localjump_error(mrb, LOCALJUMP_ERROR_RETURN);
                goto L_RAISE;
              }
              ci--;
            }
            if (ci <= cibase) { /* no jump destination */
              localjump_error(mrb, LOCALJUMP_ERROR_RETURN);
              goto L_RAISE;
            }
            ci = mrb->c->ci;
            while (cibase <= ci && ci->proc != dst) {
              CHECKPOINT_RESTORE(RBREAK_TAG_RETURN_BLOCK) {
                cibase = mrb->c->cibase;
                dst = top_proc(mrb, proc);
              }
              CHECKPOINT_MAIN(RBREAK_TAG_RETURN_BLOCK) {
                UNWIND_ENSURE(mrb, ci, pc, RBREAK_TAG_RETURN_BLOCK, proc, v);
              }
              CHECKPOINT_END(RBREAK_TAG_RETURN_BLOCK);
              ci = cipop(mrb);
              pc = ci->pc;
            }
            proc = ci->proc;
            mrb->exc = NULL; /* clear break object */
            break;
          }
          /* fallthrough */
        case OP_R_NORMAL:
        NORMAL_RETURN:
          if (ci == mrb->c->cibase) {
            struct mrb_context *c;
            c = mrb->c;

            if (!c->prev) { /* toplevel return */
              regs[irep->nlocals] = v;
              goto CHECKPOINT_LABEL_MAKE(RBREAK_TAG_STOP);
            }
            if (!c->vmexec && c->prev->ci == c->prev->cibase) {
              mrb_value exc = mrb_exc_new_lit(mrb, E_FIBER_ERROR, "double resume");
              mrb_exc_set(mrb, exc);
              goto L_RAISE;
            }
            CHECKPOINT_RESTORE(RBREAK_TAG_RETURN_TOPLEVEL) {
              c = mrb->c;
            }
            CHECKPOINT_MAIN(RBREAK_TAG_RETURN_TOPLEVEL) {
              UNWIND_ENSURE(mrb, ci, pc, RBREAK_TAG_RETURN_TOPLEVEL, proc, v);
            }
            CHECKPOINT_END(RBREAK_TAG_RETURN_TOPLEVEL);
            /* automatic yield at the end */
            c->status = MRB_FIBER_TERMINATED;
            mrb->c = c->prev;
            mrb->c->status = MRB_FIBER_RUNNING;
            c->prev = NULL;
            if (c->vmexec) {
              mrb_gc_arena_restore(mrb, ai);
              c->vmexec = FALSE;
              mrb->jmp = prev_jmp;
              return v;
            }
            ci = mrb->c->ci;
          }
          CHECKPOINT_RESTORE(RBREAK_TAG_RETURN) {
            /* do nothing */
          }
          CHECKPOINT_MAIN(RBREAK_TAG_RETURN) {
            UNWIND_ENSURE(mrb, ci, pc, RBREAK_TAG_RETURN, proc, v);
          }
          CHECKPOINT_END(RBREAK_TAG_RETURN);
          mrb->exc = NULL; /* clear break object */
          break;
        case OP_R_BREAK:
          if (MRB_PROC_STRICT_P(proc)) goto NORMAL_RETURN;
          if (MRB_PROC_ORPHAN_P(proc)) {
            mrb_value exc;

          L_BREAK_ERROR:
            exc = mrb_exc_new_lit(mrb, E_LOCALJUMP_ERROR,
                                      "break from proc-closure");
            mrb_exc_set(mrb, exc);
            goto L_RAISE;
          }
          if (!MRB_PROC_ENV_P(proc) || !MRB_ENV_ONSTACK_P(MRB_PROC_ENV(proc))) {
            goto L_BREAK_ERROR;
          }
          else {
            struct REnv *e = MRB_PROC_ENV(proc);

            if (e->cxt != mrb->c) {
              goto L_BREAK_ERROR;
            }
          }
          CHECKPOINT_RESTORE(RBREAK_TAG_BREAK) {
            /* do nothing */
          }
          CHECKPOINT_MAIN(RBREAK_TAG_BREAK) {
            UNWIND_ENSURE(mrb, ci, pc, RBREAK_TAG_BREAK, proc, v);
          }
          CHECKPOINT_END(RBREAK_TAG_BREAK);
          /* break from fiber block */
          if (ci == mrb->c->cibase && ci->pc) {
            struct mrb_context *c = mrb->c;

            mrb->c = c->prev;
            c->prev = NULL;
            ci = mrb->c->ci;
          }
          if (ci->acc < 0) {
            ci = cipop(mrb);
            mrb_gc_arena_restore(mrb, ai);
            mrb->c->vmexec = FALSE;
            mrb->exc = (struct RObject*)break_new(mrb, RBREAK_TAG_BREAK, proc, v);
            mrb->jmp = prev_jmp;
            MRB_THROW(prev_jmp);
          }
          if (FALSE) {
            struct RBreak *brk;

          L_BREAK:
            brk = (struct RBreak*)mrb->exc;
            proc = mrb_break_proc_get(brk);
            v = mrb_break_value_get(brk);
            ci = mrb->c->ci;

            switch (mrb_break_tag_get(brk)) {
#define DISPATCH_CHECKPOINTS(n, i) case n: goto CHECKPOINT_LABEL_MAKE(n);
              RBREAK_TAG_FOREACH(DISPATCH_CHECKPOINTS)
#undef DISPATCH_CHECKPOINTS
              default:
                mrb_assert(!"wrong break tag");
            }
          }
          while (mrb->c->cibase < ci && ci[-1].proc != proc->upper) {
            if (ci[-1].acc == CI_ACC_SKIP) {
              goto L_BREAK_ERROR;
            }
            CHECKPOINT_RESTORE(RBREAK_TAG_BREAK_UPPER) {
              /* do nothing */
            }
            CHECKPOINT_MAIN(RBREAK_TAG_BREAK_UPPER) {
              UNWIND_ENSURE(mrb, ci, pc, RBREAK_TAG_BREAK_UPPER, proc, v);
            }
            CHECKPOINT_END(RBREAK_TAG_BREAK_UPPER);
            ci = cipop(mrb);
            pc = ci->pc;
          }
          CHECKPOINT_RESTORE(RBREAK_TAG_BREAK_INTARGET) {
            /* do nothing */
          }
          CHECKPOINT_MAIN(RBREAK_TAG_BREAK_INTARGET) {
            UNWIND_ENSURE(mrb, ci, pc, RBREAK_TAG_BREAK_INTARGET, proc, v);
          }
          CHECKPOINT_END(RBREAK_TAG_BREAK_INTARGET);
          if (ci == mrb->c->cibase) {
            goto L_BREAK_ERROR;
          }
          mrb->exc = NULL; /* clear break object */
          break;
        default:
          /* cannot happen */
          break;
        }
        mrb_assert(ci == mrb->c->ci);
        mrb_assert(mrb->exc == NULL);

        if (mrb->c->vmexec && !mrb_vm_ci_target_class(ci)) {
          mrb_gc_arena_restore(mrb, ai);
          mrb->c->vmexec = FALSE;
          mrb->jmp = prev_jmp;
          return v;
        }
        acc = ci->acc;
        ci = cipop(mrb);
        if (acc == CI_ACC_SKIP || acc == CI_ACC_DIRECT) {
          mrb_gc_arena_restore(mrb, ai);
          mrb->jmp = prev_jmp;
          return v;
        }
        // <MASK>
        mrb_gc_arena_restore(mrb, ai);
      }
      JUMP;
    }