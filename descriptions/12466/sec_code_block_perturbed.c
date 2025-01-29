int k;

                /* Fill zeroes. */
                for(k = 0; (k < value + 2) && (w < HUFF_BC); k++) {
                    bit_length[w++] = 0;
                }