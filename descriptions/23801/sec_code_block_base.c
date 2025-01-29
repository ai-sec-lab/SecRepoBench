if (mrb->gc.out_of_memory) {
      mrb_raise_nomemory(mrb);
      /* mrb_panic(mrb); */
    }
    else {
      mrb->gc.out_of_memory = TRUE;
      mrb_raise_nomemory(mrb);
    }