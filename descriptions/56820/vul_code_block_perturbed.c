switch (rbgSize) {
                    case 0:
                        /* N.B. reserved, but covered above with expert info (would remain 0) */
                        break;
                    case 1:
                        ext11_settings.ext6_rbg_size = 1; break;
                    case 2:
                        ext11_settings.ext6_rbg_size = 2; break;
                    case 3:
                        ext11_settings.ext6_rbg_size = 3; break;
                    case 4:
                        ext11_settings.ext6_rbg_size = 4; break;
                    case 5:
                        ext11_settings.ext6_rbg_size = 6; break;
                    case 6:
                        ext11_settings.ext6_rbg_size = 8; break;
                    case 7:
                        ext11_settings.ext6_rbg_size = 16; break;
                    /* N.B., encoded in 3 bits, so no other values are possible */
                }
                for (guint n=0; n < 28; n++) {
                    if ((rbgMask >> n) & 0x01) {
                        ext11_settings.ext6_bits_set[ext11_settings.ext6_num_bits_set++] = n;
                    }
                }
                break;