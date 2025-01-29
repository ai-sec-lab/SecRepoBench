if (*option == '+')
            {
              (void) DeleteImageArtifact(_image,"identify:moments");
              break;
            }
          (void) SetImageArtifact(_image,"identify:moments","true");
          (void) SetImageArtifact(_image,"verbose","true");
          break;