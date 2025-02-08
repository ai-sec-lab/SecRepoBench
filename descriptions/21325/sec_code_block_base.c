int index_offset[2];

      /* 0 for operand A, 1 for operand B, greater for other args.  */
      int whicharg = 0;

      (*dis_info->fprintf_func)(dis_info->stream, "\t");

      maxarg = 0;

      /* First we have to find and keep track of the index bytes,
	 if we are using scaled indexed addressing mode, since the index
	 bytes occur right after the basic instruction, not as part
	 of the addressing extension.  */
      index_offset[0] = -1;
      index_offset[1] = -1;
      if (Is_gen (d[1]))
	{
	  int bitoff = d[1] == 'f' ? 10 : 5;
	  int addr_mode = bit_extract (buffer, ioffset - bitoff, 5);

	  if (Adrmod_is_index (addr_mode))
	    {
	      aoffset += 8;
	      index_offset[0] = aoffset;
	    }
	}

      if (d[2] && Is_gen (d[3]))
	{
	  int addr_mode = bit_extract (buffer, ioffset - 10, 5);

	  if (Adrmod_is_index (addr_mode))
	    {
	      aoffset += 8;
	      index_offset[1] = aoffset;
	    }
	}

      while (*d)
	{
	  argnum = *d - '1';
	  if (argnum >= MAX_ARGS)
	    abort ();
	  d++;
	  if (argnum > maxarg)
	    maxarg = argnum;
	  ioffset = print_insn_arg (*d, ioffset, &aoffset, buffer,
				    memaddr, arg_bufs[argnum],
				    whicharg > 1 ? -1 : index_offset[whicharg]);
	  d++;
	  whicharg++;
	}