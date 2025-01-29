_01 = _mm_loadl_pd(_01, (const double*)ptr);            // r0 g0 r1 g1 00 00 00 00
                if (tail > 2) {
                    _01 = _mm_loadh_pi(_01, (__m64 const* )(ptr + 4));  // r0 g0 r1 g1 r2 g2 00 00
                }