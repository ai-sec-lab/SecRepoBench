mrb_free(mrbstateptr, p);
    mrbstateptr->gc.out_of_memory = TRUE;
    mrb_raise_nomemory(mrbstateptr);