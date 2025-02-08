static void
dynapi_set_helper (void *restrict old, const Dwg_DYNAPI_field *restrict f,
                   const Dwg_Version_Type dwg_version,
                   const void *restrict value, const bool is_utf8)
{
  // TODO: sanity checks. is_malloc (TF), copy zero's (TFv)
  // if text strcpy or wcscpy, or do utf8 conversion.
  //if ((char*)old && f->is_malloc)
  //  free (old);
  if (f->is_malloc)
    {
      // NULL ptr
      if (!*(char**)value)
        memcpy (old, value, sizeof (char*));
      // fixed length (but not yet TF)
      else if (strEQc (f->type, "TFv"))
        {
          char *str = (char *)calloc (f->size, 1);
          strncpy (str, *(char**)value, f->size);
          memcpy (old, &str, sizeof (char*)); // size of ptr
        }
      // ascii
      else if (f->is_string && dwg_version < R_2007)
        {
          // FIXME: TF size calc is probably wrong
          char *str = (char *)malloc (strlen (*(char**)value)+1);
          strcpy (str, *(char**)value);
          memcpy (old, &str, sizeof (char*)); // size of ptr
        }
      // or wide
      else if (strNE (f->type, "TF") && (f->is_string && dwg_version >= R_2007))
        {
          BITCODE_TU wstr;
          if (is_utf8)
            wstr = bit_utf8_to_TU (*(char **)value, 0);
          else // source is already TU
            {
#if defined(HAVE_WCHAR_H) && defined(SIZEOF_WCHAR_T) && SIZEOF_WCHAR_T == 2
              wstr = (BITCODE_TU)malloc (2 * (wcslen (*(wchar_t **)value) + 1));
              wcscpy ((wchar_t *)wstr, *(wchar_t **)value);
#else
              int length = 0;
              for (; (*(BITCODE_TU*)value)[length]; length++)
                ;
              length++;
              wstr = (BITCODE_TU)malloc (2 * length);
              memcpy (wstr, value, length * 2);
#endif
            }
          memcpy (old, &wstr, sizeof (char*)); // size of ptr
        }
      else
        memcpy (old, value, sizeof (char*));
    }
  else
    memcpy (old, value, f->size);
}