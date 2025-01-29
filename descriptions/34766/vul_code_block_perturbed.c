Dwg_Object_BLOCK_CONTROL *_ctrl
                = ctrl->tio.object->tio.BLOCK_CONTROL;
            int at_end = 1;
            unsigned num_entries = _ctrl->num_entries;
            if (_ctrl)
              {
                for (int j = num_entries - 1; j >= 0; j--)
                  {
                    BITCODE_H ref = _ctrl->entries ? _ctrl->entries[j] : NULL;
                    if (!ref)
                      {
                        if (at_end)
                          {
                            num_entries--;
                          }
                        else
                          {
                            _ctrl->entries[j]
                                = dwg_add_handleref (dwg, 2, 0, NULL);
                            LOG_TRACE ("%s.entries[%d] = (2.0.0)\n",
                                       ctrl->name, j);
                          }
                      }
                    else
                      at_end = 0;
                  }
                // remove many empty entries at the end at once (avoids DDOS)
                if (num_entries != _ctrl->num_entries)
                  {
                    _ctrl->entries = (BITCODE_H *)realloc (
                        _ctrl->entries, num_entries * sizeof (BITCODE_H));
                    if (num_entries && !_ctrl->entries)
                      goto outofmem;
                    _ctrl->num_entries = num_entries;
                    LOG_TRACE ("%s.num_entries => %d\n", ctrl->name,
                               _ctrl->num_entries);
                  }
                // leave room for one active entry
                if (_ctrl->num_entries == 1 && !_ctrl->entries[0])
                  {
                    _ctrl->entries[0] = dwg_add_handleref (dwg, 2, 0, NULL);
                    LOG_TRACE ("%s.entries[0] = (2.0.0)\n", ctrl->name);
                  }
              }