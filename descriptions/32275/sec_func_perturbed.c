static int
dxf_objects_read (Bit_Chain *restrict dataChain, Dwg_Data *restrict dwg)
{
  char name[80];
  Dxf_Pair *pair = dxf_read_pair (dataChain);
  while (pair != NULL)
    {
      while (pair != NULL && pair->code == 0 && pair->value.s)
        {
          strncpy (name, pair->value.s, 79);
          name[79] = '\0';
          object_alias (name);
          if (is_dwg_object (name))
            {
              char *dxfname = strdup (pair->value.s);
              dxf_free_pair (pair);
              pair = new_object (name, dxfname, dataChain, dwg, 0, NULL);
              if (!pair)
                return DWG_ERR_INVALIDDWG;
            }
          else
            {
              DXF_RETURN_ENDSEC (0);
              LOG_WARN ("Unhandled 0 %s (%s)", name, "objects");
              dxf_free_pair (pair);
              pair = dxf_read_pair (dataChain);
              DXF_CHECK_EOF;
            }
        }
      dxf_free_pair (pair);
      pair = dxf_read_pair (dataChain);
      DXF_CHECK_EOF;
    }
  dxf_free_pair (pair);
  return 0;
}