JSON_TOKENS_CHECK_OVERFLOW_ERR
              if (k < (f->size / 2))
                {
                  nums[k] = (BITCODE_BS)json_long (dat, jsontokens);
                  LOG_TRACE ("%s.%s[%d]: " FORMAT_BS " [%s]\n", name, key, k,
                             nums[k], f->type);
                }
              else
                {
                  LOG_WARN ("Ignored %s.%s[%d]: %ld [%s]", name, key, k,
                             json_long (dat, jsontokens), f->type);
                }