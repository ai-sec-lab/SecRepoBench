pc = ci[0].pc;
        DEBUG(fprintf(stderr, "from :%s\n", mrb_sym_name(mrb, ci->mid)));
        proc = mrb->c->ci->proc;
        irep = proc->body.irep;
        pool = irep->pool;
        syms = irep->syms;

        regs[acc] = v;