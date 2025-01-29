if (Matrix1->InputChannels != 3 || Matrix1->OutputChannels != 3 ||
                  Matrix1->InputChannels != 3 || Matrix2->OutputChannels != 3) return FALSE;

              // Input offset should be zero
              if (Data1->Offset != NULL) return FALSE;

              // Multiply both matrices to get the result
              _cmsMAT3per(&res, (cmsMAT3*)Data2->Double, (cmsMAT3*)Data1->Double);

              // Only 2nd matrix has offset, or it is zero 
              Offset = Data2->Offset;

              // Now the result is in res + Data2 -> Offset. Maybe is a plain identity?
              if (_cmsMAT3isIdentity(&res) && Offset == NULL) {

                     // We can get rid of full matrix
                     IdentityMat = TRUE;
              }