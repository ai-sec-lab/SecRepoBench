CASE(OP_EXT1, Z) {
      instruction = READ_B();
      switch (instruction) {
#define OPCODE(instruction,ops) case OP_ ## insn: FETCH_ ## ops ## _1(); mrb->c->ci->pc = pc; goto L_OP_ ## insn ## _BODY;
#include "mruby/ops.h"
#undef OPCODE
      }
      pc--;
      NEXT;
    }
    CASE(OP_EXT2, Z) {
      instruction = READ_B();
      switch (instruction) {
#define OPCODE(instruction,ops) case OP_ ## insn: FETCH_ ## ops ## _2(); mrb->c->ci->pc = pc; goto L_OP_ ## insn ## _BODY;
#include "mruby/ops.h"
#undef OPCODE
      }
      pc--;
      NEXT;
    }
    CASE(OP_EXT3, Z) {
      uint8_t instruction = READ_B();
      switch (instruction) {
#define OPCODE(instruction,ops) case OP_ ## insn: FETCH_ ## ops ## _3(); mrb->c->ci->pc = pc; goto L_OP_ ## insn ## _BODY;
#include "mruby/ops.h"
#undef OPCODE
      }
      pc--;
      NEXT;
    }