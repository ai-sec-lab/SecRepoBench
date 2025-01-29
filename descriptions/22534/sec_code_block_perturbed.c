if (info == (unsigned char *) NULL)
                  ThrowGIFException(ResourceLimitError,
                    "MemoryAllocationFailed");
                (void) memset(info,0,reserved_length*sizeof(*info));
                for (info_length=0; ; )
                {
                  block_length=(int) ReadBlobBlock(image,info+info_length);
                  if (block_length == 0)
                    break;
                  info_length+=block_length;
                  if (info_length > (reserved_length-255))
                    {
                      reserved_length+=4096;
                      info=(unsigned char *) ResizeQuantumMemory(info,(size_t)
                        reserved_length,sizeof(*info));
                      if (info == (unsigned char *) NULL)
                        {
                          info=(unsigned char *) RelinquishMagickMemory(info);
                          ThrowGIFException(ResourceLimitError,
                            "MemoryAllocationFailed");
                        }
                    }
                }