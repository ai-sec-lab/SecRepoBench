if (irep && pc < irep->ilen && irep->debug_info) {
    mrb_irep_debug_info_file *f = get_file(irep->debug_info, pc);
    *lp = debug_get_line(mrb, f, pc);
    if (*lp > 0) {
      *fp = debug_get_filename(mrb, f);
      if (*fp) return TRUE;
    }
  }
  *lp = -1; *fp = NULL;
  return FALSE;