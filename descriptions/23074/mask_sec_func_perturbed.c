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
      // <MASK>
      add_insn_to_hash_chain (hentbuf, insn, htable, hash);
    }

  return hentbuf;
}