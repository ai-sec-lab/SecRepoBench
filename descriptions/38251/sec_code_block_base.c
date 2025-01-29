pc = ci->pc;
        DEBUG(fprintf(stderr, "from :%s\n", mrb_sym_name(mrb, ci->mid)));
        proc = ci->proc;
        irep = proc->body.irep;
        pool = irep->pool;
        syms = irep->syms;

        ci[1].stack[0] = v;