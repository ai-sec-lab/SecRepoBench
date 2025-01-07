
                PRE (R_11) // no crc16
                {
                  if (obj->size > dat->size - oldpos ||
                      obj->size + obj->address > dat->byte)
                    {
                      LOG_ERROR ("Invalid obj->size " FORMAT_RL " changed to %" PRIuSIZE,
                                 obj->size, dat->byte - oldpos);
                      error |= DWG_ERR_VALUEOUTOFBOUNDS;
                      obj->size = (dat->byte - oldpos) & 0xFFFFFFFF;
                    }
                  else if (obj->address + obj->size != dat->byte)
                    {
                      LOG_ERROR ("offset %ld",
                                 (long)(obj->address + obj->size - dat->byte));
                      if (obj->address + obj->size > dat->byte)
                        {
                          BITCODE_RL offset
                              = (BITCODE_RL)(obj->address + obj->size
                                             - dat->byte);
                          obj->num_unknown_rest = 8 * offset;
                          obj->unknown_rest = (BITCODE_TF)calloc (offset, 1);
                          if (obj->unknown_rest)
                            {
                              memcpy (obj->unknown_rest,
                                      &dat->chain[dat->byte], offset);
                              LOG_TRACE_TF (obj->unknown_rest, offset);
                            }
                          else
                            {
                              LOG_ERROR ("Out of memory");
                              obj->num_unknown_rest = 0;
                            }
                        }
                      if (obj->size > 2)
                        dat->byte = obj->address + obj->size;
                    }
                }
                LATER_VERSIONS
                {
                  if (obj->size > dat->size - oldpos ||
                      obj->size + obj->address > dat->byte)
                    {
                      LOG_ERROR ("Invalid obj->size " FORMAT_RL " changed to %" PRIuSIZE,
                                 obj->size, dat->byte + 2 - oldpos);
                      error |= DWG_ERR_VALUEOUTOFBOUNDS;
                      obj->size = ((dat->byte + 2) - oldpos) & 0xFFFFFFFF;
                    }
                  else if (obj->address + obj->size != dat->byte + 2)
                    {
                      LOG_ERROR ("offset %ld", (long)(obj->address + obj->size
                                                      - (dat->byte + 2)));
                      if (obj->address + obj->size > dat->byte + 2)
                        {
                          BITCODE_RL offset
                              = (BITCODE_RL)(obj->address + obj->size
                                             - (dat->byte + 2));
                          obj->num_unknown_rest = 8 * offset;
                          obj->unknown_rest = bit_read_TF (dat, offset);
                          if (obj->unknown_rest)
                            {
                              LOG_TRACE_TF (obj->unknown_rest, offset);
                            }
                          else
                            {
                              LOG_ERROR ("Out of memory");
                              obj->num_unknown_rest = 0;
                            }
                        }
                      if (obj->address + obj->size >= start && start > 60)
                        dat->byte = obj->address + obj->size - 2;
                    }
                  if (!bit_check_CRC (dat, obj->address, 0xC0C1))
                    error |= DWG_ERR_WRONGCRC;
                }
              