int
print_insn_rx (bfd_vma addr, disassemble_info * dis)
{
  int rv;
  RX_Data rx_data;
  RX_Opcode_Decoded opcode;
  const char * s;
  struct private priv;

  dis->private_data = (PTR) &priv;
  rx_data.pc = addr;
  rx_data.dis = dis;

  if (OPCODES_SIGSETJMP (priv.bailout) != 0)
    {
      /* Error return.  */
      return -1;
    }

  rv = rx_decode_opcode (addr, &opcode, rx_get_byte, &rx_data);

  dis->bytes_per_line = 10;

#define PR (dis->fprintf_func)
#define PS (dis->stream)
#define PC(c) PR (PS, "%c", c)

  /* Detect illegal instructions.  */
  if (opcode.op[0].size == RX_Bad_Size
      || register_names [opcode.op[0].reg] == NULL
      || register_names [opcode.op[1].reg] == NULL
      || register_names [opcode.op[2].reg] == NULL)
    {
      bfd_byte buf[10];
      int index;

      PR (PS, ".byte ");
      rx_data.dis->read_memory_func (rx_data.pc - rv, buf, rv, rx_data.dis);
      
      for (index = 0 ; index < rv; index++)
	PR (PS, "0x%02x ", buf[index]);
      return rv;
    }
      
  for (s = opcode.syntax; *s; s++)
    {
      if (*s != '%')
	{
	  PC (*s);
	}
      else
	{
	  RX_Opcode_Operand * oper;
	  int do_size = 0;
	  int do_hex = 0;
	  int do_addr = 0;

	  s ++;

	  if (*s == 'S')
	    {
	      do_size = 1;
	      s++;
	    }
	  if (*s == 'x')
	    {
	      do_hex = 1;
	      s++;
	    }
	  if (*s == 'a')
	    {
	      do_addr = 1;
	      s++;
	    }

	  switch (*s)
	    {
	    case '%':
	      PC ('%');
	      break;

	    case 's':
	      PR (PS, "%s", get_opsize_name (opcode.size));
	      break;

	    case 'b':
	      s ++;
	      if (*s == 'f')
		{
		  int imm = opcode.op[2].addend;
		  int slsb, dlsb, width;

		  dlsb = (imm >> 5) & 0x1f;
		  slsb = (imm & 0x1f);
		  slsb = (slsb >= 0x10?(slsb ^ 0x1f) + 1:slsb);
		  slsb = dlsb - slsb;
		  slsb = (slsb < 0?-slsb:slsb);
		  width = ((imm >> 10) & 0x1f) - dlsb;
		  PR (PS, "#%d, #%d, #%d, %s, %s",
		      slsb, dlsb, width,
		      get_register_name (opcode.op[1].reg),
		      get_register_name (opcode.op[0].reg));
		}
	      break;
	    // <MASK>
	    }
	}
    }

  return rv;
}