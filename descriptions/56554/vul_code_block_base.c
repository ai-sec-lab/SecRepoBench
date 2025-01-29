if (irep && pc < irep->ilen) {
    if (!irep->debug_info) {
      *lp = -1; *fp = NULL;
      return FALSE;
    }
    mrb_irep_debug_info_file *f = get_file(irep->debug_info, pc);
    *lp = debug_get_line(mrb, f, pc);
    if (*lp > 0) {
      *fp = debug_get_filename(mrb, f);
      if (*fp) return TRUE;
    }
  }
  return FALSE;