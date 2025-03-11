if (xml_start != q)
                      xml_start=(char *) ResizeQuantumMemory(xml_start,extent,sizeof(*xml_start));
                    else
                      {
                        char
                          *extent_xml;

                        extent_xml=(char *) AcquireQuantumMemory(extent,
                          sizeof(*extent_xml));
                        if (extent_xml != (char *) NULL)
                          {
                            (void) CopyMagickString(extent_xml,xml_start,extent*
                              sizeof(*extent_xml));
                            xml_start= extent_xml;
                          }
                      }
                    if (xml_start == (char *) NULL)
                      ThrowFatalException(ResourceLimitFatalError,
                        "MemoryAllocationFailed");