Dwg_Object *ctrl = &dwg->object[ctrl_id];
              char *dxfname = strdup (pair->value.s);
              BITCODE_BL idx = dwg->num_objects;
              BITCODE_H ref;
              dxf_free_pair (pair);
              // until 0 table or 0 ENDTAB
              pair = new_object (table, dxfname, datastream, dwg, ctrl_id,
                                 (BITCODE_BL *)&i);
              obj = &dwg->object[idx];