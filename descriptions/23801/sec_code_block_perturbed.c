if (mrbstateptr->gc.out_of_memory) {
      mrb_raise_nomemory(mrbstateptr);
      /* mrb_panic(mrb); */
    }
    else {
      mrbstateptr->gc.out_of_memory = TRUE;
      mrb_raise_nomemory(mrbstateptr);
    }