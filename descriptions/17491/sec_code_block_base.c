_01 = _mm_loadl_pd(_01, (const double*)ptr);            // r0 g0 r1 g1 00 00 00 00
                if (tail > 2) {
                  _01 = _mm_insert_epi16(_01, *(ptr+4), 4);             // r0 g0 r1 g1 r2 00 00 00
                  _01 = _mm_insert_epi16(_01, *(ptr+5), 5);             // r0 g0 r1 g1 r2 g2 00 00
                }