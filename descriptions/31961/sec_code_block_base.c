/* Zap data size and rep prefixes from used_prefixes and reinstate their
	 origins in all_prefixes.  */
      used_prefixes &= ~PREFIX_OPCODE;
      if (last_data_prefix >= 0)
	all_prefixes[last_data_prefix] = 0x66;
      if (last_repz_prefix >= 0)
	all_prefixes[last_repz_prefix] = 0xf3;
      if (last_repnz_prefix >= 0)
	all_prefixes[last_repnz_prefix] = 0xf2;