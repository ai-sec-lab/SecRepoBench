int _i = is2d ? j * 2 : j * 3;
                              dwg_dynapi_entity_value (_obj, obj->name,
                                                       f->name, &pts, NULL);
                              if (pair->code < 20 && pts)
                                {
                                  pts[_i] = pair->value.d;
                                }
                              else if (pair->code < 30 && pts)
                                {
                                  if (is2d)
                                    LOG_TRACE (
                                        "%s.%s[%d] = (%f, %f) [%s %d]\n", name,
                                        f->name, j, pts[_i], pair->value.d,
                                        f->type, pair->code);
                                  pts[_i + 1] = pair->value.d;
                                }
                              else if (*f->type == '3' && pts)
                                {
                                  LOG_TRACE (
                                      "%s.%s[%d] = (%f, %f, %f) [%s %d]\n",
                                      name, f->name, j, pts[_i], pts[_i + 1],
                                      pair->value.d, f->type, pair->code);
                                  pts[_i + 2] = pair->value.d;
                                  if (j == size - 1)
                                    j = 0; // restart
                                }