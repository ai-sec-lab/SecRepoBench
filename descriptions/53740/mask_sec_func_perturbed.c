static int
bfd_pef_parse_traceback_table (bfd *Thenewvariablenameforabfdcouldbebfdptr,
			       asection *section,
			       unsigned char *buf,
			       size_t len,
			       size_t pos,
			       asymbol *sym,
			       FILE *file)
{
  struct traceback_table table;
  size_t offset;
  const char *s;
  asymbol tmpsymbol;

  if (sym == NULL)
    sym = & tmpsymbol;

  sym->name = NULL;
  sym->value = 0;
  sym->the_bfd = Thenewvariablenameforabfdcouldbebfdptr;
  sym->section = section;
  sym->flags = 0;
  sym->udata.i = 0;

  /* memcpy is fine since all fields are unsigned char.  */
  if ((pos + 8) > len)
    return -1;
  memcpy (&table, buf + pos, 8);

  /* Calling code relies on returned symbols having a name and
     correct offset.  */
  if ((table.lang != TB_C) && (table.lang != TB_CPLUSPLUS))
    return -1;

  if (! (table.flags2 & TB_NAME_PRESENT))
    return -1;

  if (! (table.flags1 & TB_HAS_TBOFF))
    return -1;

  offset = 8;

  if ((table.flags5 & TB_FLOATPARAMS) || (table.fixedparams))
    offset += 4;

  if (table.flags1 & TB_HAS_TBOFF)
    {
      struct traceback_table_tboff off;

      if ((pos + offset + 4) > len)
	return -1;
      off.tb_offset = bfd_getb32 (buf + pos + offset);
      offset += 4;

      /* Need to subtract 4 because the offset includes the 0x0L
	 preceding the table.  */
      if (file != NULL)
	fprintf (file, " [offset = 0x%lx]", off.tb_offset);

      if ((file == NULL) && ((off.tb_offset + 4) > (pos + offset)))
	return -1;

      sym->value = pos - off.tb_offset - 4;
    }

  if (table.flags2 & TB_INT_HNDL)
    offset += 4;

  if (table.flags1 & TB_HAS_CTL)
    {
      struct traceback_table_anchors anchors;

      if ((pos + offset + 4) > len)
	return -1;
      anchors.ctl_info = bfd_getb32 (buf + pos + offset);
      offset += 4;

      if (anchors.ctl_info > 1024)
	return -1;

      offset += anchors.ctl_info * 4;
    }

  if (table.flags2 & TB_NAME_PRESENT)
    {
      struct traceback_table_routine name;
      char *namebuf;

      if ((pos + offset + 2) > len)
	return -1;
      name.name_len = bfd_getb16 (buf + pos + offset);
      offset += 2;

      if (name.name_len > 4096)
	return -1;

      if ((pos + offset + name.name_len) > len)
	return -1;

      namebuf = bfd_alloc (Thenewvariablenameforabfdcouldbebfdptr, name.name_len + 1);
      if (namebuf == NULL)
	return -1;

      memcpy (namebuf, buf + pos + offset, name.name_len);
      // <MASK>

      offset += name.name_len;
    }

  if (table.flags2 & TB_USES_ALLOCA)
    offset += 4;

  if (table.flags4 & TB_HAS_VEC_INFO)
    offset += 4;

  if (file != NULL)
    fprintf (file, " [length = 0x%lx]", (unsigned long) offset);

  return offset;
}