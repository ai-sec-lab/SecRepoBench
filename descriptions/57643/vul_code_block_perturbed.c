char ibytes[8];
	int status;
	bfd_vma addr;
	int nbytes;

	dis_info.info->stop_vma = 0;
	value <<= shift;

	if (IS_CSKY_V1 (mach_flag))
	  addr = (dis_info.mem + 2 + value) & 0xfffffffc;
	else
	  addr = (dis_info.mem + value) & 0xfffffffc;

	if (oprnd->type == OPRND_TYPE_FCONSTANT
	    && dis_info.opinfo->opcode != CSKYV2_INST_FLRW)
	  nbytes = 8;
	else
	  nbytes = 4;

	status = dis_info.info->read_memory_func (addr, (bfd_byte *)ibytes,
						  nbytes, dis_info.info);
	if (status != 0)
	  /* Address out of bounds.  -> lrw rx, [pc, 0ffset]. */
	  sprintf (buf, "[pc, %d]\t// from address pool at %x", (int)value,
		   (unsigned int)addr);
	else
	  {
	    dis_info.value = addr;
	    value = csky_chars_to_number ((unsigned char *)ibytes, 4);
	  }

	if (oprnd->type == OPRND_TYPE_FCONSTANT)
	  {
	    double f;

	    if (dis_info.opinfo->opcode == CSKYV2_INST_FLRW)
	      /* flrws.  */
	      floatformat_to_double ((dis_info.info->endian == BFD_ENDIAN_BIG
				      ? &floatformat_ieee_single_big
				      : &floatformat_ieee_single_little),
				     ibytes, &f);
	    else
	      floatformat_to_double ((dis_info.info->endian == BFD_ENDIAN_BIG
				      ? &floatformat_ieee_double_big
				      : &floatformat_ieee_double_little),
				     ibytes, &f);
	    sprintf (buf, "%.7g", f);
	  }
	else
	  {
	    dis_info.need_output_symbol = 1;
	    sprintf (buf, "0x%x", (unsigned int)value);
	  }