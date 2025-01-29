(t->type == JSMN_PRIMITIVE
               && f->size <= 4  // not a RS[]
               && (strEQc (f->type, "RC") || strEQc (f->type, "B")
                   || strEQc (f->type, "BB") || strEQc (f->type, "RS")
                   || strEQc (f->type, "BS") || strEQc (f->type, "RL")
                   || strEQc (f->type, "BL") || strEQc (f->type, "RSd")
                   || strEQc (f->type, "BLd") || strEQc (f->type, "BSd")))
        {
          long num = json_long (dat, tokens);
          LOG_TRACE ("%s: %ld [%s]\n", key, num, f->type)
          dwg_dynapi_header_set_value (dwg, key, &num, 0);
        }
      else if (t->type == JSMN_PRIMITIVE
               && f->size == 8 // not a RLL[]
               && (strEQc (f->type, "RLL") || strEQc (f->type, "BLL")))
        {
          uint64_t num = json_longlong (dat, tokens);
          LOG_TRACE ("%s: " FORMAT_RLL " [%s]\n", key, num, f->type)
          dwg_dynapi_header_set_value (dwg, key, &num, 0);
        }
      else if (t->type == JSMN_STRING
               && (strEQc (f->type, "TV") || strEQc (f->type, "T")))
        {
          size_t len;
          char *str = json_string (dat, tokens);
          LOG_TRACE ("%s: \"%s\" [%s]\n", key, str, f->type);
          if (dwg->header.version < R_13b1 && strEQc (key, "MENU")
              && (len = strlen (str) > 15))
            { // split into MENU + MENUEXT
              strncpy ((char *)dwg->header_vars.MENUEXT, &str[15], 45);
              str[15] = '\0';
              dwg->header_vars.MENU = strdup (str);
              dwg->header_vars.MENUEXT[45] = '\0';
            }
          else
            dwg_dynapi_header_set_value (dwg, key, &str, 1);
          free (str);
        }
      else if (t->type == JSMN_ARRAY
               && (strEQc (f->type, "3BD") || strEQc (f->type, "3RD")
                   || strEQc (f->type, "3DPOINT") || strEQc (f->type, "BE")
                   || strEQc (f->type, "3BD_1")))
        {
          BITCODE_3DPOINT pt;
          json_3DPOINT (dat, tokens, name, key, f->type, &pt);
          dwg_dynapi_header_set_value (dwg, key, &pt, 1);
        }
      else if (t->type == JSMN_ARRAY
               && (strEQc (f->type, "2BD") || strEQc (f->type, "2RD")
                   || strEQc (f->type, "2DPOINT")
                   || strEQc (f->type, "2BD_1")))
        {
          BITCODE_2DPOINT pt;
          json_2DPOINT (dat, tokens, name, key, f->type, &pt);
          dwg_dynapi_header_set_value (dwg, key, &pt, 1);
        }
      else if (t->type == JSMN_ARRAY
               && (strEQc (f->type, "BS") || strEQc (f->type, "RS")))
        {
          int size1 = t->size;
          BITCODE_BS *nums = (BITCODE_BS *)calloc (f->size, 1);
          tokens->index++;
          for (int k = 0; k < size1; k++)
            {
              JSON_TOKENS_CHECK_OVERFLOW_ERR
              nums[k] = (BITCODE_BS)json_long (dat, tokens);
              LOG_TRACE ("%s.%s[%d]: " FORMAT_BS " [%s]\n", name, key, k,
                         nums[k], f->type);
            }
          if (!size1)
            LOG_TRACE ("%s.%s: [%s] empty\n", name, key, f->type);
          if (!f->is_malloc)
            {
              dwg_dynapi_header_set_value (dwg, key, nums, 1);
              free (nums);
            }
          else
            dwg_dynapi_header_set_value (dwg, key, &nums, 1);
        }
      else if (strEQc (f->type, "TIMEBLL") || strEQc (f->type, "TIMERLL"))
        {
          BITCODE_TIMEBLL date = { 0 };
          json_TIMEBLL (dat, tokens, key, &date);
          dwg_dynapi_header_set_value (dwg, key, &date, 0);
        }
      else if (strEQc (f->type, "CMC"))
        {
          BITCODE_CMC color = { 0 };
          json_CMC (dat, dwg, tokens, name, key, &color);
          dwg_dynapi_header_set_value (dwg, key, &color, 0);
        }
      else if (t->type == JSMN_ARRAY && strEQc (f->type, "H"))
        {
          BITCODE_H hdl
              = json_HANDLE (dat, dwg, tokens, section, key, NULL, -1);
          if (hdl)
            dwg_dynapi_header_set_value (dwg, key, &hdl, 0);
        }
      //...
      else if (t->type == JSMN_OBJECT && strEQc (key, "CLASSES"))
        {
          LOG_WARN ("Unexpected next section %s", key)
          tokens->index--;
          tokens->index--;
          return 0;
        }
      else
        {
          LOG_WARN ("Unhandled %s [%s] with %s", key, f->type,
                    t_typename[t->type])
          tokens->index++;
          continue;
        }