bool
_bfd_stab_section_find_nearest_line (bfd *abfd,
				     asymbol **symbols,
				     asection *section,
				     bfd_vma offset,
				     bool *pfound,
				     const char **pfilename,
				     const char **pfnname,
				     unsigned int *pline,
				     void **pinfo)
{
  struct stab_find_info *info;
  bfd_size_type stabsize, strsize;
  bfd_byte *stab, *str;
  bfd_byte *nul_fun, *nul_str;
  bfd_size_type stroff;
  struct indexentry *indexentry;
  char *file_name;
  char *directory_name;
  bool saw_line, saw_func;

  *pfound = false;
  *pfilename = bfd_get_filename (abfd);
  *pfnname = NULL;
  *pline = 0;

  /* Stabs entries use a 12 byte format:
       4 byte string table index
       1 byte stab type
       1 byte stab other field
       2 byte stab desc field
       4 byte stab value
     FIXME: This will have to change for a 64 bit object format.

     The stabs symbols are divided into compilation units.  For the
     first entry in each unit, the type of 0, the value is the length
     of the string table for this unit, and the desc field is the
     number of stabs symbols for this unit.  */

#define STRDXOFF (0)
#define TYPEOFF (4)
#define OTHEROFF (5)
#define DESCOFF (6)
#define VALOFF (8)
#define STABSIZE (12)

  info = (struct stab_find_info *) *pinfo;
  if (info != NULL)
    {
      if (info->stabsec == NULL || info->strsec == NULL)
	{
	  /* No usable stabs debugging information.  */
	  return true;
	}

      stabsize = (info->stabsec->rawsize
		  ? info->stabsec->rawsize
		  : info->stabsec->size);
      strsize = (info->strsec->rawsize
		 ? info->strsec->rawsize
		 : info->strsec->size);
    }
  else
    {
      long reloc_size, reloc_count;
      arelent **reloc_vector;
      int i;
      char *function_name;
      bfd_size_type amt = sizeof *info;

      info = (struct stab_find_info *) bfd_zalloc (abfd, amt);
      if (info == NULL)
	return false;
      *pinfo = info;

      /* FIXME: When using the linker --split-by-file or
	 --split-by-reloc options, it is possible for the .stab and
	 .stabstr sections to be split.  We should handle that.  */

      info->stabsec = bfd_get_section_by_name (abfd, ".stab");
      info->strsec = bfd_get_section_by_name (abfd, ".stabstr");

      if (info->stabsec == NULL || info->strsec == NULL)
	{
	  /* Try SOM section names.  */
	  info->stabsec = bfd_get_section_by_name (abfd, "$GDB_SYMBOLS$");
	  info->strsec  = bfd_get_section_by_name (abfd, "$GDB_STRINGS$");

	  if (info->stabsec == NULL || info->strsec == NULL)
	    return true;
	}

      stabsize = (info->stabsec->rawsize
		  ? info->stabsec->rawsize
		  : info->stabsec->size);
      stabsize = (stabsize / STABSIZE) * STABSIZE;
      strsize = (info->strsec->rawsize
		 ? info->strsec->rawsize
		 : info->strsec->size);

      // <MASK>
    }

  /* We are passed a section relative offset.  The offsets in the
     stabs information are absolute.  */
  offset += bfd_section_vma (section);

#ifdef ENABLE_CACHING
  if (info->cached_indexentry != NULL
      && offset >= info->cached_offset
      && offset < (info->cached_indexentry + 1)->val)
    {
      stab = info->cached_stab;
      indexentry = info->cached_indexentry;
      file_name = info->cached_file_name;
    }
  else
#endif
    {
      long low, high;
      long mid = -1;

      /* Cache non-existent or invalid.  Do binary search on
	 indextable.  */
      indexentry = NULL;

      low = 0;
      high = info->indextablesize - 1;
      while (low != high)
	{
	  mid = (high + low) / 2;
	  if (offset >= info->indextable[mid].val
	      && offset < info->indextable[mid + 1].val)
	    {
	      indexentry = &info->indextable[mid];
	      break;
	    }

	  if (info->indextable[mid].val > offset)
	    high = mid;
	  else
	    low = mid + 1;
	}

      if (indexentry == NULL)
	return true;

      stab = indexentry->stab + STABSIZE;
      file_name = indexentry->file_name;
    }

  directory_name = indexentry->directory_name;
  str = indexentry->str;

  saw_line = false;
  saw_func = false;
  for (; stab < (indexentry+1)->stab; stab += STABSIZE)
    {
      bool done;
      bfd_vma val;

      done = false;

      switch (stab[TYPEOFF])
	{
	case N_SOL:
	  /* The name of an include file.  */
	  val = bfd_get_32 (abfd, stab + VALOFF);
	  if (val <= offset)
	    {
	      file_name = (char *) str + bfd_get_32 (abfd, stab + STRDXOFF);
	      if (file_name >= (char *) info->strs + strsize
		  || file_name < (char *) str)
		file_name = NULL;
	      *pline = 0;
	    }
	  break;

	case N_SLINE:
	case N_DSLINE:
	case N_BSLINE:
	  /* A line number.  If the function was specified, then the value
	     is relative to the start of the function.  Otherwise, the
	     value is an absolute address.  */
	  val = ((indexentry->function_name ? indexentry->val : 0)
		 + bfd_get_32 (abfd, stab + VALOFF));
	  /* If this line starts before our desired offset, or if it's
	     the first line we've been able to find, use it.  The
	     !saw_line check works around a bug in GCC 2.95.3, which emits
	     the first N_SLINE late.  */
	  if (!saw_line || val <= offset)
	    {
	      *pline = bfd_get_16 (abfd, stab + DESCOFF);

#ifdef ENABLE_CACHING
	      info->cached_stab = stab;
	      info->cached_offset = val;
	      info->cached_file_name = file_name;
	      info->cached_indexentry = indexentry;
#endif
	    }
	  if (val > offset)
	    done = true;
	  saw_line = true;
	  break;

	case N_FUN:
	case N_SO:
	  if (saw_func || saw_line)
	    done = true;
	  saw_func = true;
	  break;
	}

      if (done)
	break;
    }

  *pfound = true;

  if (file_name == NULL || IS_ABSOLUTE_PATH (file_name)
      || directory_name == NULL)
    *pfilename = file_name;
  else
    {
      size_t dirlen;

      dirlen = strlen (directory_name);
      if (info->filename == NULL
	  || filename_ncmp (info->filename, directory_name, dirlen) != 0
	  || filename_cmp (info->filename + dirlen, file_name) != 0)
	{
	  size_t len;

	  /* Don't free info->filename here.  objdump and other
	     apps keep a copy of a previously returned file name
	     pointer.  */
	  len = strlen (file_name) + 1;
	  info->filename = (char *) bfd_alloc (abfd, dirlen + len);
	  if (info->filename == NULL)
	    return false;
	  memcpy (info->filename, directory_name, dirlen);
	  memcpy (info->filename + dirlen, file_name, len);
	}

      *pfilename = info->filename;
    }

  if (indexentry->function_name != NULL)
    {
      char *s;

      /* This will typically be something like main:F(0,1), so we want
	 to clobber the colon.  It's OK to change the name, since the
	 string is in our own local storage anyhow.  */
      s = strchr (indexentry->function_name, ':');
      if (s != NULL)
	*s = '\0';

      *pfnname = indexentry->function_name;
    }

  return true;
}