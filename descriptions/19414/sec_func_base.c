static int
print_insn_mode (const char *d,
		 int size,
		 unsigned char *p0,
		 bfd_vma addr,	/* PC for this arg to be relative to.  */
		 disassemble_info *info)
{
  unsigned char *p = p0;
  unsigned char mode, reg;

  /* Fetch and interpret mode byte.  */
  mode = (unsigned char) NEXTBYTE (p);
  reg = mode & 0xF;
  switch (mode & 0xF0)
    {
    case 0x00:
    case 0x10:
    case 0x20:
    case 0x30: /* Literal mode			$number.  */
      if (d[1] == 'd' || d[1] == 'f' || d[1] == 'g' || d[1] == 'h')
	(*info->fprintf_func) (info->stream, "$0x%x [%c-float]", mode, d[1]);
      else
        (*info->fprintf_func) (info->stream, "$0x%x", mode);
      break;
    case 0x40: /* Index:			base-addr[Rn] */
      {
	unsigned char *q = p0 + 1;
	unsigned char nextmode = NEXTBYTE (q);
	if (nextmode < 0x60 || nextmode == 0x8f)
	  /* Literal, index, register, or immediate is invalid.  In
	     particular don't recurse into another index mode which
	     might overflow the_buffer.   */
	  (*info->fprintf_func) (info->stream, "[invalid base]");
	else
	  p += print_insn_mode (d, size, p0 + 1, addr + 1, info);
	(*info->fprintf_func) (info->stream, "[%s]", reg_names[reg]);
      }
      break;
    case 0x50: /* Register:			Rn */
      (*info->fprintf_func) (info->stream, "%s", reg_names[reg]);
      break;
    case 0x60: /* Register deferred:		(Rn) */
      (*info->fprintf_func) (info->stream, "(%s)", reg_names[reg]);
      break;
    case 0x70: /* Autodecrement:		-(Rn) */
      (*info->fprintf_func) (info->stream, "-(%s)", reg_names[reg]);
      break;
    case 0x80: /* Autoincrement:		(Rn)+ */
      if (reg == 0xF)
	{	/* Immediate?  */
	  int i;

	  FETCH_DATA (info, p + size);
	  (*info->fprintf_func) (info->stream, "$0x");
	  if (d[1] == 'd' || d[1] == 'f' || d[1] == 'g' || d[1] == 'h')
	    {
	      int float_word;

	      float_word = p[0] | (p[1] << 8);
	      if ((d[1] == 'd' || d[1] == 'f')
		  && (float_word & 0xff80) == 0x8000)
		{
		  (*info->fprintf_func) (info->stream, "[invalid %c-float]",
					 d[1]);
		}
	      else
		{
	          for (i = 0; i < size; i++)
		    (*info->fprintf_func) (info->stream, "%02x",
		                           p[size - i - 1]);
	          (*info->fprintf_func) (info->stream, " [%c-float]", d[1]);
		}
	    }
	  else
	    {
	      for (i = 0; i < size; i++)
	        (*info->fprintf_func) (info->stream, "%02x", p[size - i - 1]);
	    }
	  p += size;
	}
      else
	(*info->fprintf_func) (info->stream, "(%s)+", reg_names[reg]);
      break;
    case 0x90: /* Autoincrement deferred:	@(Rn)+ */
      if (reg == 0xF)
	(*info->fprintf_func) (info->stream, "*0x%x", NEXTLONG (p));
      else
	(*info->fprintf_func) (info->stream, "@(%s)+", reg_names[reg]);
      break;
    case 0xB0: /* Displacement byte deferred:	*displ(Rn).  */
      (*info->fprintf_func) (info->stream, "*");
      /* Fall through.  */
    case 0xA0: /* Displacement byte:		displ(Rn).  */
      if (reg == 0xF)
	(*info->print_address_func) (addr + 2 + NEXTBYTE (p), info);
      else
	(*info->fprintf_func) (info->stream, "0x%x(%s)", NEXTBYTE (p),
			       reg_names[reg]);
      break;
    case 0xD0: /* Displacement word deferred:	*displ(Rn).  */
      (*info->fprintf_func) (info->stream, "*");
      /* Fall through.  */
    case 0xC0: /* Displacement word:		displ(Rn).  */
      if (reg == 0xF)
	(*info->print_address_func) (addr + 3 + NEXTWORD (p), info);
      else
	(*info->fprintf_func) (info->stream, "0x%x(%s)", NEXTWORD (p),
			       reg_names[reg]);
      break;
    case 0xF0: /* Displacement long deferred:	*displ(Rn).  */
      (*info->fprintf_func) (info->stream, "*");
      /* Fall through.  */
    case 0xE0: /* Displacement long:		displ(Rn).  */
      if (reg == 0xF)
	(*info->print_address_func) (addr + 5 + NEXTLONG (p), info);
      else
	(*info->fprintf_func) (info->stream, "0x%x(%s)", NEXTLONG (p),
			       reg_names[reg]);
      break;
    }

  return p - p0;
}