{
      s->as.heap.aux.capa = shared->capa;
      s->as.heap.ptr[s->as.heap.len] = '\0';
      mrb_free(mrb, shared);
    }
    else {
      str_init_modifiable(mrb, s, s->as.heap.ptr, (size_t)s->as.heap.len);
      str_decref(mrb, shared);
    }