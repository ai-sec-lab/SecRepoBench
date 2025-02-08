char buf [4];
      unsigned long value;
      const CGEN_INSN *insn = &insns[i];

      if (! (* cd->dis_hash_p) (insn))
	continue;

      /* We don't know whether the target uses the buffer or the base insn
	 to hash on, so set both up.  */

      value = CGEN_INSN_BASE_VALUE (insn);
      bfd_put_bits ((bfd_vma) value,
		    buf,
		    CGEN_INSN_MASK_BITSIZE (insn),
		    big_p);
      hash = (* cd->dis_hash) (buf, value);