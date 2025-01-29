if (target_channels == 1)
                  SetPixelGray(image,target_scale*QuantumRange*(*p),q);
                else
                  SetPixelRed(image,target_scale*QuantumRange*(*p),q);
                p++;
                if (target_channels > 1)
                  {
                    SetPixelGreen(image,target_scale*QuantumRange*(*p),q);
                    p++;
                    SetPixelBlue(image,target_scale*QuantumRange*(*p),q);
                    p++;
                  }
                if (target_channels > 3)
                  {
                    SetPixelBlack(image,target_scale*QuantumRange*(*p),q);
                    p++;
                  }