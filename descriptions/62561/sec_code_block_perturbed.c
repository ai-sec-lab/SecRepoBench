BITCODE_BL idx = drawing->num_objects;
          // LOG_HANDLE ("dxfname = strdup (%s)\n", dxfname);
          if (idx)
            {
              Dwg_Object *obj = &drawing->object[drawing->num_objects - 1];
              if (!obj->handle.value)
                {
                  BITCODE_RLL next_handle = dwg_next_handle (drawing);
                  dwg_add_handle (&obj->handle, 0, next_handle, NULL);
                  LOG_TRACE ("%s.handle = (0.%d." FORMAT_RLLx ")\n", obj->name,
                             obj->handle.size, obj->handle.value);
                }
            }
          dxf_free_pair (pair);
          pair = new_object (name, dxfname, dat, drawing, 0, NULL);
          if (!pair)
            {
              Dwg_Object *obj = &drawing->object[idx];
              free (dxfname);
              if (idx != drawing->num_objects)
                obj->dxfname = NULL;
              return DWG_ERR_INVALIDDWG;
            }