if (strEQc (f->type, "TFv"))
        {
          char *str = (char *)calloc (f->size + 1, 1);
          strncpy (str, *(char**)value, f->size);
          // we copy just the pointer to heap-alloced str, not the string
          free (*(char **)old);
          memcpy (old, &str, sizeof (char*)); // size of ptr
        }
      // ascii
      else if (f->is_string && dwg_version < R_2007)
        {
          // FIXME: TF size calc is probably wrong
          size_t len = strlen (*(char**)value);
          char *str = (char *)malloc (len + 1);
          memcpy (str, *(char**)value, len + 1);
          // we copy just the pointer, not the string
          free (*(char **)old);
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
#ifdef HAVE_NATIVE_WCHAR2
              wstr = (BITCODE_TU)malloc (2 * (wcslen (*(wchar_t **)value) + 1));
              wcscpy ((wchar_t *)wstr, *(wchar_t **)value);
#else
              int length = 0;
              for (; (*(BITCODE_TU*)value)[length]; length++)
                ;
              length++;
              wstr = (BITCODE_TU)malloc ((2 * length) + 1);
              memcpy (wstr, value, length * 2);
#endif
            }
          free (*(char **)old);
          memcpy (old, &wstr, sizeof (char*)); // size of ptr
        }
      else
        memcpy (old, value, sizeof (char*));