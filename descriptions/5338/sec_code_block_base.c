if (target_channels == 1)
                  SetPixelGray(image,ClampToQuantum(target_scale*
                    QuantumRange*(*p)),q);
                else
                  SetPixelRed(image,ClampToQuantum(target_scale*
                    QuantumRange*(*p)),q);
                p++;
                if (target_channels > 1)
                  {
                    SetPixelGreen(image,ClampToQuantum(target_scale*
                      QuantumRange*(*p)),q);
                    p++;
                    SetPixelBlue(image,ClampToQuantum(target_scale*
                      QuantumRange*(*p)),q);
                    p++;
                  }
                if (target_channels > 3)
                  {
                    SetPixelBlack(image,ClampToQuantum(target_scale*
                      QuantumRange*(*p)),q);
                    p++;
                  }