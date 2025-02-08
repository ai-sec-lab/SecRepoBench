MRB_USE_BIGINT
        {
          const char *s = pool[b].u.str;
          regs[a] = mrb_bint_new_str(mrb, s+2, (uint8_t)s[0], s[1]);
        }
        break;
#else
        goto L_INT_OVERFLOW;