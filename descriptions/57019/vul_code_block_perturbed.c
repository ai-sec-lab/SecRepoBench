if (*option == '+')
            {
              (void) DeleteImageArtifact(_image,"identify:moments");
              break;
            }
          (void) SetImageArtifact(_image,"identify:moments",arg1);
          (void) SetImageArtifact(_image,"verbose","true");
          break;