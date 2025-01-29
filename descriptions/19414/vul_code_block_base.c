p += print_insn_mode (d, size, p0 + 1, addr + 1, info);
      (*info->fprintf_func) (info->stream, "[%s]", reg_names[reg]);
      break;