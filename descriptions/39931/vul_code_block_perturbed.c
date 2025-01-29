if (argcount < 0) {
      mrb_raisef(mrb, E_ARGUMENT_ERROR, "negative argc for funcall (%i)", argcount);
    }
    c = mrb_class(mrb, self);
    m = mrb_method_search_vm(mrb, &c, mid);
    mrb_stack_extend(mrb, argcount + 3);
    if (MRB_METHOD_UNDEF_P(m) || argcount >= 15) {
      mrb_value args = mrb_ary_new_from_values(mrb, argcount, argv);

      ci->stack[n+1] = args;
      argcount = 15;
    }