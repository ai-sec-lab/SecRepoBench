/*
** vm.c - virtual machine for mruby
**
** See Copyright Notice in mruby.h
*/

#include <stddef.h>
#include <stdarg.h>
#ifndef MRB_NO_FLOAT
#include <math.h>
#endif
#include <mruby.h>
#include <mruby/array.h>
#include <mruby/class.h>
#include <mruby/hash.h>
#include <mruby/irep.h>
#include <mruby/numeric.h>
#include <mruby/proc.h>
#include <mruby/range.h>
#include <mruby/string.h>
#include <mruby/variable.h>
#include <mruby/error.h>
#include <mruby/opcode.h>
#include "value_array.h"
#include <mruby/throw.h>
#include <mruby/dump.h>
#include <mruby/presym.h>

#ifdef MRB_NO_STDIO
#if defined(__cplusplus)
extern "C" {
#endif
void abort(void);
#if defined(__cplusplus)
}  /* extern "C" { */
#endif
#endif

#define STACK_INIT_SIZE 128
#define CALLINFO_INIT_SIZE 32

/* Define amount of linear stack growth. */
#ifndef MRB_STACK_GROWTH
#define MRB_STACK_GROWTH 128
#endif

/* Maximum mrb_funcall() depth. Should be set lower on memory constrained systems. */
#ifndef MRB_FUNCALL_DEPTH_MAX
#define MRB_FUNCALL_DEPTH_MAX 512
#endif

/* Maximum stack depth. Should be set lower on memory constrained systems.
The value below allows about 60000 recursive calls in the simplest case. */
#ifndef MRB_STACK_MAX
#define MRB_STACK_MAX (0x40000 - MRB_STACK_GROWTH)
#endif

#ifdef VM_DEBUG
# define DEBUG(x) (x)
#else
# define DEBUG(x)
#endif


#ifndef MRB_GC_FIXED_ARENA
static void
mrb_gc_arena_shrink(mrb_state *mrb, int idx)
{
  mrb_gc *gc = &mrb->gc;
  int capa = gc->arena_capa;

  if (idx < capa / 4) {
    capa >>= 2;
    if (capa < MRB_GC_ARENA_SIZE) {
      capa = MRB_GC_ARENA_SIZE;
    }
    if (capa != gc->arena_capa) {
      gc->arena = (struct RBasic**)mrb_realloc(mrb, gc->arena, sizeof(struct RBasic*)*capa);
      gc->arena_capa = capa;
    }
  }
}
#else
#define mrb_gc_arena_shrink(mrb,idx)
#endif

#define CALL_MAXARGS 127

void mrb_method_missing(mrb_state *mrb, mrb_sym name, mrb_value self, mrb_value args);

static inline void
stack_clear(mrb_value *from, size_t count)
{
#ifdef MRB_NAN_BOXING
  while (count-- > 0) {
    SET_NIL_VALUE(*from);
    from++;
  }
#else
  memset(from, 0, sizeof(mrb_value)*count);
#endif
}

static inline void
stack_copy(mrb_value *dst, const mrb_value *src, size_t size)
{
  memcpy(dst, src, sizeof(mrb_value)*size);
}

static void
stack_init(mrb_state *mrb)
{
  struct mrb_context *c = mrb->c;

  /* mrb_assert(mrb->stack == NULL); */
  c->stbase = (mrb_value *)mrb_calloc(mrb, STACK_INIT_SIZE, sizeof(mrb_value));
  c->stend = c->stbase + STACK_INIT_SIZE;

  /* mrb_assert(ci == NULL); */
  c->cibase = (mrb_callinfo *)mrb_calloc(mrb, CALLINFO_INIT_SIZE, sizeof(mrb_callinfo));
  c->ciend = c->cibase + CALLINFO_INIT_SIZE;
  c->ci = c->cibase;
  c->ci->u.target_class = mrb->object_class;
  c->ci->stack = c->stbase;
}

static inline void
envadjust(mrb_state *mrb, mrb_value *oldbase, mrb_value *newbase, size_t oldsize)
{
  mrb_callinfo *ci = mrb->c->cibase;

  if (newbase == oldbase) return;
  while (ci <= mrb->c->ci) {
    struct REnv *e = mrb_vm_ci_env(ci);
    mrb_value *st;

    if (e && MRB_ENV_ONSTACK_P(e) &&
        (st = e->stack) && oldbase <= st && st < oldbase+oldsize) {
      ptrdiff_t off = e->stack - oldbase;

      e->stack = newbase + off;
    }

    if (ci->proc && MRB_PROC_ENV_P(ci->proc) && e != MRB_PROC_ENV(ci->proc)) {
      e = MRB_PROC_ENV(ci->proc);

      if (e && MRB_ENV_ONSTACK_P(e) &&
          (st = e->stack) && oldbase <= st && st < oldbase+oldsize) {
        ptrdiff_t off = e->stack - oldbase;

        e->stack = newbase + off;
      }
    }

    ci->stack = newbase + (ci->stack - oldbase);
    ci++;
  }
}

/** def rec ; $deep =+ 1 ; if $deep > 1000 ; return 0 ; end ; rec ; end  */

static void
stack_extend_alloc(mrb_state *mrb, mrb_int room)
{
  mrb_value *oldbase = mrb->c->stbase;
  mrb_value *newstack;
  size_t oldsize = mrb->c->stend - mrb->c->stbase;
  size_t size = oldsize;
  size_t off = mrb->c->ci->stack ? mrb->c->stend - mrb->c->ci->stack : 0;

  if (off > size) size = off;
#ifdef MRB_STACK_EXTEND_DOUBLING
  if ((size_t)room <= size)
    size *= 2;
  else
    size += room;
#else
  /* Use linear stack growth.
     It is slightly slower than doubling the stack space,
     but it saves memory on small devices. */
  if (room <= MRB_STACK_GROWTH)
    size += MRB_STACK_GROWTH;
  else
    size += room;
#endif

  newstack = (mrb_value *)mrb_realloc_simple(mrb, mrb->c->stbase, sizeof(mrb_value) * size);
  if (newstack == NULL) {
    mrb_exc_raise(mrb, mrb_obj_value(mrb->stack_err));
  }
  stack_clear(&(newstack[oldsize]), size - oldsize);
  envadjust(mrb, oldbase, newstack, oldsize);
  mrb->c->stbase = newstack;
  mrb->c->stend = mrb->c->stbase + size;

  /* Raise an exception if the new stack size will be too large,
     to prevent infinite recursion. However, do this only after resizing the stack, so mrb_raise has stack space to work with. */
  if (size > MRB_STACK_MAX) {
    mrb_exc_raise(mrb, mrb_obj_value(mrb->stack_err));
  }
}

MRB_API void
mrb_stack_extend(mrb_state *mrb, mrb_int room)
{
  if (!mrb->c->ci->stack || mrb->c->ci->stack + room >= mrb->c->stend) {
    stack_extend_alloc(mrb, room);
  }
}

static inline struct REnv*
uvenv(mrb_state *mrb, mrb_int up)
{
  const struct RProc *proc = mrb->c->ci->proc;
  struct REnv *e;

  while (up--) {
    proc = proc->upper;
    if (!proc) return NULL;
  }
  e = MRB_PROC_ENV(proc);
  if (e) return e;              /* proc has enclosed env */
  else {
    mrb_callinfo *ci = mrb->c->ci;
    mrb_callinfo *cb = mrb->c->cibase;

    while (cb <= ci) {
      if (ci->proc == proc) {
        return mrb_vm_ci_env(ci);
      }
      ci--;
    }
  }
  return NULL;
}

static inline const struct RProc*
top_proc(mrb_state *mrb, const struct RProc *proc)
{
  while (proc->upper) {
    if (MRB_PROC_SCOPE_P(proc) || MRB_PROC_STRICT_P(proc))
      return proc;
    proc = proc->upper;
  }
  return proc;
}

#define CI_ACC_SKIP    -1
#define CI_ACC_DIRECT  -2
#define CI_ACC_RESUMED -3

static inline mrb_callinfo*
cipush(mrb_state *mrb, mrb_int push_stacks, mrb_int acc,
       struct RClass *target_class, const struct RProc *proc, mrb_sym mid, mrb_int argc)
{
  struct mrb_context *c = mrb->c;
  mrb_callinfo *ci = c->ci;

  if (ci + 1 == c->ciend) {
    ptrdiff_t size = ci - c->cibase;

    c->cibase = (mrb_callinfo *)mrb_realloc(mrb, c->cibase, sizeof(mrb_callinfo)*size*2);
    c->ci = c->cibase + size;
    c->ciend = c->cibase + size * 2;
  }
  ci = ++c->ci;
  ci->mid = mid;
  mrb_vm_ci_proc_set(ci, proc);
  ci->stack = ci[-1].stack + push_stacks;
  ci->argc = (int16_t)argc;
  ci->acc = (int16_t)acc;
  ci->u.target_class = target_class;

  return ci;
}

void
mrb_env_unshare(mrb_state *mrb, struct REnv *e)
{
  if (e == NULL) return;
  else {
    size_t len = (size_t)MRB_ENV_LEN(e);
    mrb_value *p;

    if (!MRB_ENV_ONSTACK_P(e)) return;
    if (e->cxt != mrb->c) return;
    if (e == mrb_vm_ci_env(mrb->c->cibase)) return; /* for mirb */
    p = (mrb_value *)mrb_malloc(mrb, sizeof(mrb_value)*len);
    if (len > 0) {
      stack_copy(p, e->stack, len);
    }
    e->stack = p;
    MRB_ENV_CLOSE(e);
    mrb_write_barrier(mrb, (struct RBasic *)e);
  }
}

static inline mrb_callinfo*
cipop(mrb_state *mrb)
{
  struct mrb_context *c = mrb->c;
  struct REnv *env = mrb_vm_ci_env(c->ci);

  c->ci--;
  if (env) mrb_env_unshare(mrb, env);
  return c->ci;
}

MRB_API mrb_value
mrb_protect_error(mrb_state *mrb, mrb_protect_error_func *body, void *userdata, mrb_bool *error)
{
  struct mrb_jmpbuf *prev_jmp = mrb->jmp;
  struct mrb_jmpbuf c_jmp;
  mrb_value result = mrb_nil_value();
  int ai = mrb_gc_arena_save(mrb);
  const struct mrb_context *c = mrb->c;
  int ci_index = c->ci - c->cibase;

  if (error) { *error = FALSE; }

  MRB_TRY(&c_jmp) {
    mrb->jmp = &c_jmp;
    result = body(mrb, userdata);
    mrb->jmp = prev_jmp;
  }
  MRB_CATCH(&c_jmp) {
    mrb->jmp = prev_jmp;
    result = mrb_obj_value(mrb->exc);
    mrb->exc = NULL;
    if (error) { *error = TRUE; }
    if (mrb->c == c) {
      while (c->ci - c->cibase > ci_index) {
        cipop(mrb);
      }
    }
    else {
      // It was probably switched by mrb_fiber_resume().
      // Simply destroy all successive CI_ACC_DIRECTs once the fiber has been switched.
      c = mrb->c;
      while (c->ci > c->cibase && c->ci->acc == CI_ACC_DIRECT) {
        cipop(mrb);
      }
    }
  }
  MRB_END_EXC(&c_jmp);

  mrb_gc_arena_restore(mrb, ai);
  mrb_gc_protect(mrb, result);
  return result;
}

void mrb_exc_set(mrb_state *mrb, mrb_value exc);
static mrb_value mrb_run(mrb_state *mrb, const struct RProc* proc, mrb_value self);

#ifndef MRB_FUNCALL_ARGC_MAX
#define MRB_FUNCALL_ARGC_MAX 16
#endif

MRB_API mrb_value
mrb_funcall(mrb_state *mrb, mrb_value self, const char *name, mrb_int argc, ...)
{
  mrb_value argv[MRB_FUNCALL_ARGC_MAX];
  va_list ap;
  mrb_int i;
  mrb_sym mid = mrb_intern_cstr(mrb, name);

  if (argc > MRB_FUNCALL_ARGC_MAX) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "Too long arguments. (limit=" MRB_STRINGIZE(MRB_FUNCALL_ARGC_MAX) ")");
  }

  va_start(ap, argc);
  for (i = 0; i < argc; i++) {
    argv[i] = va_arg(ap, mrb_value);
  }
  va_end(ap);
  return mrb_funcall_argv(mrb, self, mid, argc, argv);
}

MRB_API mrb_value
mrb_funcall_id(mrb_state *mrb, mrb_value self, mrb_sym mid, mrb_int argc, ...)
{
  mrb_value argv[MRB_FUNCALL_ARGC_MAX];
  va_list ap;
  mrb_int i;

  if (argc > MRB_FUNCALL_ARGC_MAX) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "Too long arguments. (limit=" MRB_STRINGIZE(MRB_FUNCALL_ARGC_MAX) ")");
  }

  va_start(ap, argc);
  for (i = 0; i < argc; i++) {
    argv[i] = va_arg(ap, mrb_value);
  }
  va_end(ap);
  return mrb_funcall_argv(mrb, self, mid, argc, argv);
}

static mrb_int
ci_nregs(mrb_callinfo *ci)
{
  const struct RProc *p;
  mrb_int n = 0;

  if (!ci) return 3;
  p = ci->proc;
  if (!p) {
    if (ci->argc < 0) return 3;
    return ci->argc+2;
  }
  if (!MRB_PROC_CFUNC_P(p) && p->body.irep) {
    n = p->body.irep->nregs;
  }
  if (ci->argc < 0) {
    if (n < 3) n = 3; /* self + args + blk */
  }
  if (ci->argc > n) {
    n = ci->argc + 2; /* self + blk */
  }
  return n;
}

MRB_API mrb_value
mrb_funcall_with_block(mrb_state *mrb, mrb_value self, mrb_sym mid, mrb_int argc, const mrb_value *argv, mrb_value blk)
{
  mrb_value val;
  int ai = mrb_gc_arena_save(mrb);

  if (!mrb->jmp) {
    struct mrb_jmpbuf c_jmp;
    ptrdiff_t nth_ci = mrb->c->ci - mrb->c->cibase;

    MRB_TRY(&c_jmp) {
      mrb->jmp = &c_jmp;
      /* recursive call */
      val = mrb_funcall_with_block(mrb, self, mid, argc, argv, blk);
      mrb->jmp = 0;
    }
    MRB_CATCH(&c_jmp) { /* error */
      while (nth_ci < (mrb->c->ci - mrb->c->cibase)) {
        cipop(mrb);
      }
      mrb->jmp = 0;
      val = mrb_obj_value(mrb->exc);
    }
    MRB_END_EXC(&c_jmp);
    mrb->jmp = 0;
  }
  else {
    mrb_method_t m;
    struct RClass *c;
    mrb_callinfo *ci;
    mrb_int n = ci_nregs(mrb->c->ci);
    ptrdiff_t voff = -1;

    if (!mrb->c->stbase) {
      stack_init(mrb);
    }
    if (argc < 0) {
      mrb_raisef(mrb, E_ARGUMENT_ERROR, "negative argc for funcall (%i)", argc);
    }
    c = mrb_class(mrb, self);
    m = mrb_method_search_vm(mrb, &c, mid);
    if (MRB_METHOD_UNDEF_P(m)) {
      mrb_sym missing = MRB_SYM(method_missing);
      mrb_value args = mrb_ary_new_from_values(mrb, argc, argv);
      m = mrb_method_search_vm(mrb, &c, missing);
      if (MRB_METHOD_UNDEF_P(m)) {
        mrb_method_missing(mrb, mid, self, args);
      }
      mrb_ary_unshift(mrb, args, mrb_symbol_value(mid));
      mrb_stack_extend(mrb, n+2);
      mrb->c->ci->stack[n+1] = args;
      argc = -1;
    }
    if (mrb->c->ci - mrb->c->cibase > MRB_FUNCALL_DEPTH_MAX) {
      mrb_exc_raise(mrb, mrb_obj_value(mrb->stack_err));
    }
    ci = cipush(mrb, n, 0, c, NULL, mid, argc);
    if (argc < 0) argc = 1;
    if (mrb->c->stbase <= argv && argv < mrb->c->stend) {
      voff = argv - mrb->c->stbase;
    }
    if (argc >= CALL_MAXARGS) {
      mrb_value args = mrb_ary_new_from_values(mrb, argc, argv);

      mrb->c->ci->stack[1] = args;
      ci->argc = -1;
      argc = 1;
    }
    mrb_stack_extend(mrb, argc + 2);
    if (MRB_METHOD_PROC_P(m)) {
      struct RProc *p = MRB_METHOD_PROC(m);

      mrb_vm_ci_proc_set(ci, p);
      if (!MRB_PROC_CFUNC_P(p)) {
        mrb_stack_extend(mrb, p->body.irep->nregs + argc);
      }
    }
    if (voff >= 0) {
      argv = mrb->c->stbase + voff;
    }
    mrb->c->ci->stack[0] = self;
    if (ci->argc > 0) {
      stack_copy(mrb->c->ci->stack+1, argv, argc);
    }
    mrb->c->ci->stack[argc+1] = blk;

    if (MRB_METHOD_CFUNC_P(m)) {
      ci->acc = CI_ACC_DIRECT;
      val = MRB_METHOD_CFUNC(m)(mrb, self);
      cipop(mrb);
    }
    else {
      ci->acc = CI_ACC_SKIP;
      val = mrb_run(mrb, MRB_METHOD_PROC(m), self);
    }
  }
  mrb_gc_arena_restore(mrb, ai);
  mrb_gc_protect(mrb, val);
  return val;
}

MRB_API mrb_value
mrb_funcall_argv(mrb_state *mrb, mrb_value self, mrb_sym mid, mrb_int argc, const mrb_value *argv)
{
  return mrb_funcall_with_block(mrb, self, mid, argc, argv, mrb_nil_value());
}

static mrb_value
exec_irep(mrb_state *mrb, mrb_value self, struct RProc *p)
{
  mrb_callinfo *ci = mrb->c->ci;
  int keep, nregs;

  ci->stack[0] = self;
  mrb_vm_ci_proc_set(ci, p);
  if (MRB_PROC_CFUNC_P(p)) {
    return MRB_PROC_CFUNC(p)(mrb, self);
  }
  nregs = p->body.irep->nregs;
  if (ci->argc < 0) keep = 3;
  else keep = ci->argc + 2;
  if (nregs < keep) {
    mrb_stack_extend(mrb, keep);
  }
  else {
    mrb_stack_extend(mrb, nregs);
    stack_clear(ci->stack+keep, nregs-keep);
  }

  cipush(mrb, 0, 0, NULL, NULL, 0, 0);

  return self;
}

mrb_value
mrb_exec_irep(mrb_state *mrb, mrb_value self, struct RProc *p)
{
  mrb_callinfo *ci = mrb->c->ci;
  if (ci->acc >= 0) {
    return exec_irep(mrb, self, p);
  }
  else {
    mrb_value ret;
    if (MRB_PROC_CFUNC_P(p)) {
      cipush(mrb, 0, CI_ACC_DIRECT, mrb_vm_ci_target_class(ci), p, ci->mid, ci->argc);
      ret = MRB_PROC_CFUNC(p)(mrb, self);
      cipop(mrb);
    }
    else {
      int keep = (ci->argc < 0 ? 1 : ci->argc) + 2 /* receiver + block */;
      ret = mrb_top_run(mrb, p, self, keep);
    }
    if (mrb->exc && mrb->jmp) {
      mrb_exc_raise(mrb, mrb_obj_value(mrb->exc));
    }
    return ret;
  }
}

/* 15.3.1.3.4  */
/* 15.3.1.3.44 */
/*
 *  call-seq:
 *     obj.send(symbol [, args...])        -> obj
 *     obj.__send__(symbol [, args...])      -> obj
 *
 *  Invokes the method identified by _symbol_, passing it any
 *  arguments specified. You can use <code>__send__</code> if the name
 *  +send+ clashes with an existing method in _obj_.
 *
 *     class Klass
 *       def hello(*args)
 *         "Hello " + args.join(' ')
 *       end
 *     end
 *     k = Klass.new
 *     k.send :hello, "gentle", "readers"   #=> "Hello gentle readers"
 */
mrb_value
mrb_f_send(mrb_state *mrb, mrb_value self)
{
  mrb_sym name;
  mrb_value block, *regs;
  const mrb_value *argv;
  mrb_int argc, i, len;
  mrb_method_t m;
  struct RClass *c;
  mrb_callinfo *ci;

  mrb_get_args(mrb, "n*&", &name, &argv, &argc, &block);
  ci = mrb->c->ci;
  if (ci->acc < 0) {
  funcall:
    return mrb_funcall_with_block(mrb, self, name, argc, argv, block);
  }

  c = mrb_class(mrb, self);
  m = mrb_method_search_vm(mrb, &c, name);
  if (MRB_METHOD_UNDEF_P(m)) {            /* call method_mising */
    goto funcall;
  }

  ci->mid = name;
  ci->u.target_class = c;
  regs = mrb->c->ci->stack+1;
  /* remove first symbol from arguments */
  if (ci->argc >= 0) {
    for (i=0,len=ci->argc; i<len; i++) {
      regs[i] = regs[i+1];
    }
    ci->argc--;
  }
  else {                     /* variable length arguments */
    regs[0] = mrb_ary_subseq(mrb, regs[0], 1, RARRAY_LEN(regs[0]) - 1);
  }

  if (MRB_METHOD_CFUNC_P(m)) {
    if (MRB_METHOD_PROC_P(m)) {
      mrb_vm_ci_proc_set(ci, MRB_METHOD_PROC(m));
    }
    return MRB_METHOD_CFUNC(m)(mrb, self);
  }
  return exec_irep(mrb, self, MRB_METHOD_PROC(m));
}

static mrb_value
eval_under(mrb_state *mrb, mrb_value self, mrb_value blk, struct RClass *c)
{
  struct RProc *p;
  mrb_callinfo *ci;
  int nregs;

  if (mrb_nil_p(blk)) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "no block given");
  }
  ci = mrb->c->ci;
  if (ci->acc == CI_ACC_DIRECT) {
    return mrb_yield_with_class(mrb, blk, 1, &self, self, c);
  }
  ci->u.target_class = c;
  p = mrb_proc_ptr(blk);
  mrb_vm_ci_proc_set(ci, p);
  ci->argc = 1;
  ci->mid = ci[-1].mid;
  if (MRB_PROC_CFUNC_P(p)) {
    mrb_stack_extend(mrb, 3);
    mrb->c->ci->stack[0] = self;
    mrb->c->ci->stack[1] = self;
    mrb->c->ci->stack[2] = mrb_nil_value();
    return MRB_PROC_CFUNC(p)(mrb, self);
  }
  nregs = p->body.irep->nregs;
  if (nregs < 3) nregs = 3;
  mrb_stack_extend(mrb, nregs);
  mrb->c->ci->stack[0] = self;
  mrb->c->ci->stack[1] = self;
  stack_clear(mrb->c->ci->stack+2, nregs-2);
  ci = cipush(mrb, 0, 0, NULL, NULL, 0, 0);

  return self;
}

/* 15.2.2.4.35 */
/*
 *  call-seq:
 *     mod.class_eval {| | block }  -> obj
 *     mod.module_eval {| | block } -> obj
 *
 *  Evaluates block in the context of _mod_. This can
 *  be used to add methods to a class. <code>module_eval</code> returns
 *  the result of evaluating its argument.
 */
mrb_value
mrb_mod_module_eval(mrb_state *mrb, mrb_value mod)
{
  mrb_value a, b;

  if (mrb_get_args(mrb, "|S&", &a, &b) == 1) {
    mrb_raise(mrb, E_NOTIMP_ERROR, "module_eval/class_eval with string not implemented");
  }
  return eval_under(mrb, mod, b, mrb_class_ptr(mod));
}

/* 15.3.1.3.18 */
/*
 *  call-seq:
 *     obj.instance_eval {| | block }                       -> obj
 *
 *  Evaluates the given block,within  the context of the receiver (_obj_).
 *  In order to set the context, the variable +self+ is set to _obj_ while
 *  the code is executing, giving the code access to _obj_'s
 *  instance variables. In the version of <code>instance_eval</code>
 *  that takes a +String+, the optional second and third
 *  parameters supply a filename and starting line number that are used
 *  when reporting compilation errors.
 *
 *     class KlassWithSecret
 *       def initialize
 *         @secret = 99
 *       end
 *     end
 *     k = KlassWithSecret.new
 *     k.instance_eval { @secret }   #=> 99
 */
mrb_value
mrb_obj_instance_eval(mrb_state *mrb, mrb_value self)
{
  mrb_value a, b;

  if (mrb_get_args(mrb, "|S&", &a, &b) == 1) {
    mrb_raise(mrb, E_NOTIMP_ERROR, "instance_eval with string not implemented");
  }
  return eval_under(mrb, self, b, mrb_singleton_class_ptr(mrb, self));
}

MRB_API mrb_value
mrb_yield_with_class(mrb_state *mrb, mrb_value b, mrb_int argc, const mrb_value *argv, mrb_value self, struct RClass *c)
{
  struct RProc *p;
  mrb_sym mid = mrb->c->ci->mid;
  mrb_callinfo *ci;
  mrb_value val;
  mrb_int n;

  if (mrb_nil_p(b)) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "no block given");
  }
  ci = mrb->c->ci;
  n = ci_nregs(ci);
  if (ci - mrb->c->cibase > MRB_FUNCALL_DEPTH_MAX) {
    mrb_exc_raise(mrb, mrb_obj_value(mrb->stack_err));
  }
  p = mrb_proc_ptr(b);
  ci = cipush(mrb, n, CI_ACC_SKIP, c, p, mid, 0 /* dummy */);
  if (argc >= CALL_MAXARGS) {
    ci->argc = -1;
    n = 3;
  }
  else {
    ci->argc = (int)argc;
    n = argc + 2;
  }
  mrb_stack_extend(mrb, n);
  mrb->c->ci->stack[0] = self;
  if (ci->argc < 0) {
    mrb->c->ci->stack[1] = mrb_ary_new_from_values(mrb, argc, argv);
    argc = 1;
  }
  else if (argc > 0) {
    stack_copy(mrb->c->ci->stack+1, argv, argc);
  }
  mrb->c->ci->stack[argc+1] = mrb_nil_value();

  if (MRB_PROC_CFUNC_P(p)) {
    ci->acc = CI_ACC_DIRECT;
    val = MRB_PROC_CFUNC(p)(mrb, self);
    cipop(mrb);
  }
  else {
    val = mrb_run(mrb, p, self);
  }
  return val;
}

MRB_API mrb_value
mrb_yield_argv(mrb_state *mrb, mrb_value b, mrb_int argc, const mrb_value *argv)
{
  struct RProc *p = mrb_proc_ptr(b);

  return mrb_yield_with_class(mrb, b, argc, argv, MRB_PROC_ENV(p)->stack[0], MRB_PROC_TARGET_CLASS(p));
}

MRB_API mrb_value
mrb_yield(mrb_state *mrb, mrb_value b, mrb_value arg)
{
  struct RProc *p = mrb_proc_ptr(b);

  return mrb_yield_with_class(mrb, b, 1, &arg, MRB_PROC_ENV(p)->stack[0], MRB_PROC_TARGET_CLASS(p));
}

mrb_value
mrb_yield_cont(mrb_state *mrb, mrb_value b, mrb_value self, mrb_int argc, const mrb_value *argv)
{
  struct RProc *p;
  mrb_callinfo *ci;

  if (mrb_nil_p(b)) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "no block given");
  }
  if (!mrb_proc_p(b)) {
    mrb_raise(mrb, E_TYPE_ERROR, "not a block");
  }

  p = mrb_proc_ptr(b);
  ci = mrb->c->ci;

  mrb_stack_extend(mrb, 3);
  mrb->c->ci->stack[1] = mrb_ary_new_from_values(mrb, argc, argv);
  mrb->c->ci->stack[2] = mrb_nil_value();
  ci->argc = -1;
  return exec_irep(mrb, self, p);
}

static struct RBreak*
break_new(mrb_state *mrb, uint32_t tag, const struct RProc *p, mrb_value val)
{
  struct RBreak *brk;

  brk = MRB_OBJ_ALLOC(mrb, MRB_TT_BREAK, NULL);
  mrb_break_proc_set(brk, p);
  mrb_break_value_set(brk, val);
  mrb_break_tag_set(brk, tag);

  return brk;
}

#define MRB_CATCH_FILTER_RESCUE (UINT32_C(1) << MRB_CATCH_RESCUE)
#define MRB_CATCH_FILTER_ENSURE (UINT32_C(1) << MRB_CATCH_ENSURE)
#define MRB_CATCH_FILTER_ALL    (MRB_CATCH_FILTER_RESCUE | MRB_CATCH_FILTER_ENSURE)

static const struct mrb_irep_catch_handler *
catch_handler_find(mrb_state *mrb, mrb_callinfo *ci, const mrb_code *pc, uint32_t filter)
{
  const mrb_irep *irep;
  ptrdiff_t xpc;
  size_t cnt;
  const struct mrb_irep_catch_handler *e;

/* The comparison operators use `>` and `<=` because pc already points to the next instruction */
#define catch_cover_p(pc, beg, end) ((pc) > (ptrdiff_t)(beg) && (pc) <= (ptrdiff_t)(end))

  if (ci->proc == NULL || MRB_PROC_CFUNC_P(ci->proc)) return NULL;
  irep = ci->proc->body.irep;
  if (irep->clen < 1) return NULL;
  xpc = pc - irep->iseq;
  /* If it retry at the top level, pc will be 0, so check with -1 as the start position */
  mrb_assert(catch_cover_p(xpc, -1, irep->ilen));
  if (!catch_cover_p(xpc, -1, irep->ilen)) return NULL;

  /* Currently uses a simple linear search to avoid processing complexity. */
  cnt = irep->clen;
  e = mrb_irep_catch_handler_table(irep) + cnt - 1;
  for (; cnt > 0; cnt --, e --) {
    if (((UINT32_C(1) << e->type) & filter) &&
        catch_cover_p(xpc, mrb_irep_catch_handler_unpack(e->begin), mrb_irep_catch_handler_unpack(e->end))) {
      return e;
    }
  }

#undef catch_cover_p

  return NULL;
}

typedef enum {
  LOCALJUMP_ERROR_RETURN = 0,
  LOCALJUMP_ERROR_BREAK = 1,
  LOCALJUMP_ERROR_YIELD = 2
} localjump_error_kind;

static void
localjump_error(mrb_state *mrb, localjump_error_kind kind)
{
  char kind_str[3][7] = { "return", "break", "yield" };
  char kind_str_len[] = { 6, 5, 5 };
  static const char lead[] = "unexpected ";
  mrb_value msg;
  mrb_value exc;

  msg = mrb_str_new_capa(mrb, sizeof(lead) + 7);
  mrb_str_cat(mrb, msg, lead, sizeof(lead) - 1);
  mrb_str_cat(mrb, msg, kind_str[kind], kind_str_len[kind]);
  exc = mrb_exc_new_str(mrb, E_LOCALJUMP_ERROR, msg);
  mrb_exc_set(mrb, exc);
}

static void
argnum_error(mrb_state *mrb, mrb_int num)
{
  mrb_value exc;
  mrb_value str;
  mrb_int argc = mrb->c->ci->argc;

  if (argc < 0) {
    mrb_value args = mrb->c->ci->stack[1];
    if (mrb_array_p(args)) {
      argc = RARRAY_LEN(args);
    }
  }
  if (mrb->c->ci->mid) {
    str = mrb_format(mrb, "'%n': wrong number of arguments (%i for %i)",
                     mrb->c->ci->mid, argc, num);
  }
  else {
    str = mrb_format(mrb, "wrong number of arguments (%i for %i)", argc, num);
  }
  exc = mrb_exc_new_str(mrb, E_ARGUMENT_ERROR, str);
  mrb_exc_set(mrb, exc);
}

static mrb_bool
break_tag_p(struct RBreak *brk, uint32_t tag)
{
  return (brk != NULL && brk->tt == MRB_TT_BREAK) ? TRUE : FALSE;
}

static void
prepare_tagged_break(mrb_state *mrb, uint32_t tag, const struct RProc *proc, mrb_value val)
{
  if (break_tag_p((struct RBreak*)mrb->exc, tag)) {
    mrb_break_tag_set((struct RBreak*)mrb->exc, tag);
  }
  else {
    mrb->exc = (struct RObject*)break_new(mrb, tag, proc, val);
  }
}

#define THROW_TAGGED_BREAK(mrb, tag, proc, val) \
  do { \
    prepare_tagged_break(mrb, tag, proc, val); \
    goto L_CATCH_TAGGED_BREAK; \
  } while (0)

#define UNWIND_ENSURE(mrb, ci, pc, tag, proc, val) \
  do { \
    ch = catch_handler_find(mrb, ci, pc, MRB_CATCH_FILTER_ENSURE); \
    if (ch) { \
      THROW_TAGGED_BREAK(mrb, tag, proc, val); \
    } \
  } while (0)

/*
 *  CHECKPOINT_RESTORE(tag) {
 *    This part is executed when jumping by the same "tag" of RBreak (it is not executed the first time).
 *    Write the code required (initialization of variables, etc.) for the subsequent processing.
 *  }
 *  CHECKPOINT_MAIN(tag) {
 *    This part is always executed.
 *  }
 *  CHECKPOINT_END(tag);
 *
 *  ...
 *
 *  // Jump to CHECKPOINT_RESTORE with the same "tag".
 *  goto CHECKPOINT_LABEL_MAKE(tag);
 */

#define CHECKPOINT_LABEL_MAKE(tag) L_CHECKPOINT_ ## tag

#define CHECKPOINT_RESTORE(tag) \
  do { \
    if (FALSE) { \
      CHECKPOINT_LABEL_MAKE(tag): \
      do {

#define CHECKPOINT_MAIN(tag) \
      } while (0); \
    } \
    do {

#define CHECKPOINT_END(tag) \
    } while (0); \
  } while (0)

#ifdef MRB_USE_DEBUG_HOOK
#define CODE_FETCH_HOOK(mrb, irep, pc, regs) if ((mrb)->code_fetch_hook) (mrb)->code_fetch_hook((mrb), (irep), (pc), (regs));
#else
#define CODE_FETCH_HOOK(mrb, irep, pc, regs)
#endif

#ifdef MRB_BYTECODE_DECODE_OPTION
#define BYTECODE_DECODER(x) ((mrb)->bytecode_decoder)?(mrb)->bytecode_decoder((mrb), (x)):(x)
#else
#define BYTECODE_DECODER(x) (x)
#endif

#ifndef MRB_NO_DIRECT_THREADING
#if defined __GNUC__ || defined __clang__ || defined __INTEL_COMPILER
#define DIRECT_THREADED
#endif
#endif /* ifndef MRB_NO_DIRECT_THREADING */

#ifndef DIRECT_THREADED

#define INIT_DISPATCH for (;;) { insn = BYTECODE_DECODER(*pc); CODE_FETCH_HOOK(mrb, irep, pc, regs); switch (insn) {
#define CASE(insn,ops) case insn: pc++; FETCH_ ## ops (); mrb->c->ci->pc = pc; L_ ## insn ## _BODY:
#define NEXT goto L_END_DISPATCH
#define JUMP NEXT
#define END_DISPATCH L_END_DISPATCH:;}}

#else

#define INIT_DISPATCH JUMP; return mrb_nil_value();
#define CASE(insn,ops) L_ ## insn: pc++; FETCH_ ## ops (); mrb->c->ci->pc = pc; L_ ## insn ## _BODY:
#define NEXT insn=BYTECODE_DECODER(*pc); CODE_FETCH_HOOK(mrb, irep, pc, regs); goto *optable[insn]
#define JUMP NEXT

#define END_DISPATCH

#endif

MRB_API mrb_value
mrb_vm_run(mrb_state *mrb, const struct RProc *proc, mrb_value self, mrb_int stack_keep)
{
  const mrb_irep *irep = proc->body.irep;
  mrb_value result;
  struct mrb_context *c = mrb->c;
  ptrdiff_t cioff = c->ci - c->cibase;
  mrb_int nregs = irep->nregs;

  if (!c->stbase) {
    stack_init(mrb);
  }
  if (stack_keep > nregs)
    nregs = stack_keep;
  mrb_stack_extend(mrb, nregs);
  stack_clear(c->ci->stack + stack_keep, nregs - stack_keep);
  c->ci->stack[0] = self;
  result = mrb_vm_exec(mrb, proc, irep->iseq);
  if (mrb->c != c) {
    if (mrb->c->fib) {
      mrb_write_barrier(mrb, (struct RBasic*)mrb->c->fib);
    }
    mrb->c = c;
  }
  else if (c->ci - c->cibase > cioff) {
    c->ci = c->cibase + cioff;
  }
  return result;
}

static mrb_bool
check_target_class(mrb_state *mrb)
{
  if (!mrb_vm_ci_target_class(mrb->c->ci)) {
    mrb_value exc = mrb_exc_new_lit(mrb, E_TYPE_ERROR, "no target class or module");
    mrb_exc_set(mrb, exc);
    return FALSE;
  }
  return TRUE;
}

mrb_value
get_send_args(mrb_state *mrb, mrb_int argc, mrb_value *regs)
{
  if (argc < 0) return regs[0];
  return mrb_ary_new_from_values(mrb, argc, regs);
}

static void
proc_adjust_upper(struct RProc *p)
{
  /* skip upper procs while unnamed blocks and method closures */
  while (p->upper) {
    if (MRB_FLAG_TEST(p->upper, MRB_PROC_SCOPE) &&
        !MRB_FLAG_TEST(p->upper, MRB_PROC_STRICT)) {
      break;
    }
    p->upper = p->upper->upper;
  }
}

mrb_value mrb_obj_missing(mrb_state *mrb, mrb_value mod);
void mrb_hash_check_kdict(mrb_state *mrb, mrb_value self);
void mrb_method_added(mrb_state *mrb, struct RClass *c, mrb_sym mid);

MRB_API mrb_value
mrb_vm_exec(mrb_state *mrb, const struct RProc *proc, const mrb_code *pc)
{
// <MASK>
}

static mrb_value
mrb_run(mrb_state *mrb, const struct RProc *proc, mrb_value self)
{
  if (mrb->c->ci->argc < 0) {
    return mrb_vm_run(mrb, proc, self, 3); /* receiver, args and block) */
  }
  else {
    return mrb_vm_run(mrb, proc, self, mrb->c->ci->argc + 2); /* argc + 2 (receiver and block) */
  }
}

MRB_API mrb_value
mrb_top_run(mrb_state *mrb, const struct RProc *proc, mrb_value self, mrb_int stack_keep)
{
  mrb_value v;

  if (!mrb->c->cibase) {
    return mrb_vm_run(mrb, proc, self, stack_keep);
  }
  if (mrb->c->ci == mrb->c->cibase) {
    mrb_vm_ci_env_set(mrb->c->ci, NULL);
    return mrb_vm_run(mrb, proc, self, stack_keep);
  }
  cipush(mrb, 0, CI_ACC_SKIP, mrb->object_class, NULL, 0, 0);
  v = mrb_vm_run(mrb, proc, self, stack_keep);

  return v;
}

#if defined(MRB_USE_CXX_EXCEPTION) && defined(__cplusplus)
# if !defined(MRB_USE_CXX_ABI)
} /* end of extern "C" */
# endif
mrb_int mrb_jmpbuf::jmpbuf_id = 0;
# if !defined(MRB_USE_CXX_ABI)
extern "C" {
# endif
#endif
