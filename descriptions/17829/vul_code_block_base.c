{
            _01 = _mm_setzero_si128();
            if (tail > 1) {
                _01 = _mm_loadl_pd(_01, (const double*)ptr);            // r0 g0 r1 g1 00 00 00 00
                if (tail > 2) {
                  _01 = _mm_insert_epi16(_01, *(ptr+4), 4);             // r0 g0 r1 g1 r2 00 00 00
                  _01 = _mm_insert_epi16(_01, *(ptr+5), 5);             // r0 g0 r1 g1 r2 g2 00 00
                }
            } else {
                _01 = _mm_loadl_pi(_01, (__m64 const*)ptr + 0);         // r0 g0 00 00 00 00 00 00
            }
        } else {
            _01 = _mm_loadu_si128(((__m128i*)ptr) + 0);  // r0 g0 r1 g1 r2 g2 r3 g3
        }