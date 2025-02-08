static CGEN_INSN_LIST *
hash_insn_array (CGEN_CPU_DESC cd,
		 const CGEN_INSN * insns,
		 int count,
		 int entsize UNUSEDATTRIBUTE,
		 CGEN_INSN_LIST ** htable,
		 CGEN_INSN_LIST * hentbuf)
{
  int big_p = CGEN_CPU_INSN_ENDIAN (cd) == CGEN_ENDIAN_BIG;
  int i;

  for (i = count - 1; i >= 0; --i, ++hentbuf)
    {
      unsigned int hash;
      char buf [8];
      unsigned long value;
      const CGEN_INSN *insn = &insns[i];
      size_t size;

      if (! (* cd->dis_hash_p) (insn))
	continue;

      /* We don't know whether the target uses the buffer or the base insn
	 to hash on, so set both up.  */

      value = CGEN_INSN_BASE_VALUE (insn);
      size = CGEN_INSN_MASK_BITSIZE (insn);
      OPCODES_ASSERT (size <= sizeof (buf) * 8);
      bfd_put_bits ((bfd_vma) value, buf, size, big_p);
      hash = (* cd->dis_hash) (buf, value);
      add_insn_to_hash_chain (hentbuf, insn, htable, hash);
    }

  return hentbuf;
}