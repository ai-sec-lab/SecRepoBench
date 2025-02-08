static void
dynapi_set_helper (void *restrict old, const Dwg_DYNAPI_field *restrict f,
                   const Dwg_Version_Type dwg_version,
                   const void *restrict val, const bool is_utf8)
{
  // TODO: sanity checks. is_malloc (TF), copy zero's (TFv)
  // if text strcpy or wcscpy, or do utf8 conversion.
  //if ((char*)old && f->is_malloc)
  //  free (old);
  if (f->is_malloc)
    {
      // NULL ptr
      if (!*(char**)val)
        memcpy (old, val, sizeof (char*));
      // fixed length (but not yet TF)
      else if // <MASK>
      else if (f->is_string && dwg_version < R_2007)
        {
          // FIXME: TF size calc is probably wrong
          char *str = (char *)malloc (strlen (*(char**)val)+1);
          strcpy (str, *(char**)val);
          memcpy (old, &str, sizeof (char*)); // size of ptr
        }
      // or wide
      else if (strNE (f->type, "TF") && (f->is_string && dwg_version >= R_2007))
        {
          BITCODE_TU wstr;
          if (is_utf8)
            wstr = bit_utf8_to_TU (*(char **)val, 0);
          else // source is already TU
            {
#if defined(HAVE_WCHAR_H) && defined(SIZEOF_WCHAR_T) && SIZEOF_WCHAR_T == 2
              wstr = (BITCODE_TU)malloc (2 * (wcslen (*(wchar_t **)val) + 1));
              wcscpy ((wchar_t *)wstr, *(wchar_t **)val);
#else
              int length = 0;
              for (; (*(BITCODE_TU*)val)[length]; length++)
                ;
              length++;
              wstr = (BITCODE_TU)malloc (2 * length);
              memcpy (wstr, val, length * 2);
#endif
            }
          memcpy (old, &wstr, sizeof (char*)); // size of ptr
        }
      else
        memcpy (old, val, sizeof (char*));
    }
  else
    memcpy (old, val, f->size);
}