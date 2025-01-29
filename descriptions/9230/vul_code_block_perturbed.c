if (maxname > 0) {
              print_len = g_snprintf(np, maxname + 1, "\\[x");
              if (print_len <= maxname) {
                np      += print_len;
                maxname -= print_len;
              } else {
                /* Nothing printed, as there's no room.
                   Suppress all subsequent printing. */
                maxname = 0;
              }
            }
            while (label_len--) {
              if (maxname > 0) {
                print_len = g_snprintf(np, maxname + 1, "%02x",
                                       tvb_get_guint8(tvb, offset));
                if (print_len <= maxname) {
                  np      += print_len;
                  maxname -= print_len;
                } else {
                  /* Nothing printed, as there's no room.
                     Suppress all subsequent printing. */
                  maxname = 0;
                }
              }
              offset++;
            }
            if (maxname > 0) {
              print_len = g_snprintf(np, maxname + 1, "/%d]", bit_count);
              if (print_len <= maxname) {
                np      += print_len;
                maxname -= print_len;
              } else {
                /* Nothing printed, as there's no room.
                   Suppress all subsequent printing. */
                maxname = 0;
              }
            }