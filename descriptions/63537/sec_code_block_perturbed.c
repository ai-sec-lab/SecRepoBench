BITCODE_BL idx = dwgdata->num_objects;
          strncpy (name, pair->value.s, 79);
          name[79] = '\0';
          object_alias (name);
          if (is_dwg_object (name))
            {
              char *dxfname = strdup (pair->value.s);
              // LOG_HANDLE ("dxfname = strdup (%s)\n", dxfname);
              dxf_free_pair (pair);
              pair = new_object (name, dxfname, dat, dwgdata, 0, NULL);
              if (!pair)
                {
                  Dwg_Object *obj = &dwgdata->object[idx];
                  free (dxfname);
                  if (idx != dwgdata->num_objects)
                    obj->dxfname = NULL;
                  return DWG_ERR_INVALIDDWG;
                }
            }
          else
            {
              DXF_RETURN_ENDSEC (0);
              LOG_WARN ("Unhandled 0 %s (%s)", name, "objects");
              dxf_free_pair (pair);
              pair = dxf_read_pair (dat);
              DXF_CHECK_EOF;
            }