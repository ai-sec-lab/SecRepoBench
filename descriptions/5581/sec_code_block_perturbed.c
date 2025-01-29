if (p != q)
                      p=(char *) ResizeQuantumMemory(p,extent,sizeof(*p));
                    else
                      {
                        char
                          *extent_xml;

                        extent_xml=(char *) AcquireQuantumMemory(extent,
                          sizeof(*extent_xml));
                        if (extent_xml != (char *) NULL)
                          {
                            ResetMagickMemory(extent_xml,0,extent*
                              sizeof(*extent_xml));
                            (void) CopyMagickString(extent_xml,p,extent*
                              sizeof(*extent_xml));
                          }
                        p=extent_xml;
                      }
                    if (p == (char *) NULL)
                      ThrowFatalException(ResourceLimitFatalError,
                        "MemoryAllocationFailed");
                    xml=p+offset;