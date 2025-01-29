mrb_free(mrb, p);
    mrb->gc.out_of_memory = TRUE;
    mrb_raise_nomemory(mrb);