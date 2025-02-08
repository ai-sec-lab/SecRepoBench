{
      rstring->as.heap.aux.capa = shared->capa;
      rstring->as.heap.ptr[rstring->as.heap.len] = '\0';
      mrb_free(mrb, shared);
    }
    else {
      str_init_modifiable(mrb, rstring, rstring->as.heap.ptr, (size_t)rstring->as.heap.len);
      str_decref(mrb, shared);
    }