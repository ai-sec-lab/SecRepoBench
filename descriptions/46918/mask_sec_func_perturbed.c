static int
json_HEADER (Bit_Chain *restrict bitchain, Dwg_Data *restrict dwg,
             jsmntokens_t *restrict tokens)
{
  const char *section = "HEADER";
  const char *name = section;
  jsmntok_t *t = &tokens->tokens[tokens->index];
  //Dwg_Header_Variables *_obj = &dwg->header_vars;
  Dwg_Object *obj = NULL;
  int size = t->size;

  if (t->type != JSMN_OBJECT)
    {
      LOG_ERROR ("Unexpected %s at %u of %ld tokens, expected %s OBJECT",
                 t_typename[t->type], tokens->index, tokens->num_tokens,
                 section);
      json_advance_unknown (bitchain, tokens, t->type, 0);
      return DWG_ERR_INVALIDTYPE;
    }
  LOG_TRACE ("\n%s pos:%d [%d keys]\n--------------------\n", section,
             tokens->index, t->size);
  tokens->index++;
  for (int i = 0; i < size; i++)
    {
      char key[80];
      Dwg_DYNAPI_field *f;

      json_fixed_key (key, bitchain, tokens);
      JSON_TOKENS_CHECK_OVERFLOW_ERR
      t = &tokens->tokens[tokens->index];
      f = (Dwg_DYNAPI_field *)dwg_dynapi_header_field (key);
      if (!f)
        {
          LOG_WARN ("Unknown key HEADER.%s", key)
          json_advance_unknown (bitchain, tokens, t->type, 0);
          continue;
        }
      else if (t->type == JSMN_PRIMITIVE
               && (strEQc (f->type, "BD") || strEQc (f->type, "RD")))
        // <MASK>
    }
  LOG_TRACE ("End of %s\n", section)
  // the key
  tokens->index--;
  return 0;
}