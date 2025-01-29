{
	unsigned char *q = p0 + 1;
	unsigned char nextmode = NEXTBYTE (q);
	if (nextmode < 0x60 || nextmode == 0x8f)
	  /* Literal, index, register, or immediate is invalid.  In
	     particular don't recurse into another index mode which
	     might overflow the_buffer.   */
	  (*info->fprintf_func) (info->stream, "[invalid base]");
	else
	  p += print_insn_mode (d, size, p0 + 1, pcaddress + 1, info);
	(*info->fprintf_func) (info->stream, "[%s]", reg_names[reg]);
      }
      break;