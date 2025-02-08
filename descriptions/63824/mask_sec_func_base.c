static int
json_HEADER (Bit_Chain *restrict dat, Dwg_Data *restrict dwg,
             jsmntokens_t *restrict tokens)
{
  const char *section = "HEADER";
  const char *name = section;
  jsmntok_t *t = &tokens->tokens[tokens->index];
  // Dwg_Header_Variables *_obj = &dwg->header_vars;
  Dwg_Object *obj = NULL;
  int size = t->size;

  if (t->type != JSMN_OBJECT)
    {
      LOG_ERROR ("Unexpected %s at %u of %ld tokens, expected %s OBJECT",
                 t_typename[t->type], tokens->index, tokens->num_tokens,
                 section);
      json_advance_unknown (dat, tokens, t->type, 0);
      return DWG_ERR_INVALIDTYPE;
    }
  LOG_TRACE ("\n%s pos:%d [%d keys]\n--------------------\n", section,
             tokens->index, t->size);
  tokens->index++;
  for (int i = 0; i < size; i++)
    {
      char key[80];
      Dwg_DYNAPI_field *f;

      json_fixed_key (key, dat, tokens);
      JSON_TOKENS_CHECK_OVERFLOW_ERR
      t = &tokens->tokens[tokens->index];
      f = (Dwg_DYNAPI_field *)dwg_dynapi_header_field (key);
      if (!f)
        {
          if (t->type == JSMN_ARRAY && strEQc (key, "layer_colors"))
            {
              tokens->index++;
              for (int index = 0; index < MAX (t->size, 128); index++)
                {
                  dwg->header_vars.layer_colors[index]
                      = (BITCODE_RS)json_long (dat, tokens);
                  LOG_TRACE ("%s: " FORMAT_RS " [RS]\n", key,
                             dwg->header_vars.layer_colors[index]);
                }
              JSON_TOKENS_CHECK_OVERFLOW_ERR;
            }
          else
            {
              LOG_WARN ("Unknown key HEADER.%s", key)
              json_advance_unknown (dat, tokens, t->type, 0);
              continue;
            }
        }
      else if (t->type == JSMN_PRIMITIVE
               && (strEQc (f->type, "BD") || strEQc (f->type, "RD")))
        {
          double num = json_float (dat, tokens);
          LOG_TRACE ("%s: " FORMAT_RD " [%s]\n", key, num, f->type)
          dwg_dynapi_header_set_value (dwg, key, &num, 0);
        }
      else if // <MASK>
    }
  LOG_TRACE ("End of %s\n", section)
  // the key
  tokens->index--;
  return 0;
}