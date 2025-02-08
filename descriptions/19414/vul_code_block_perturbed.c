p += print_insn_mode (d, size, p0 + 1, pcaddress + 1, info);
      (*info->fprintf_func) (info->stream, "[%s]", reg_names[reg]);
      break;