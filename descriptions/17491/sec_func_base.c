SI void load2(const uint16_t* ptr, size_t tail, U16* r, U16* g) {
        __m128i _01;
        if (__builtin_expect(tail,0)) {
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
        auto rg01_23 = _mm_shufflelo_epi16(_01, 0xD8);      // r0 r1 g0 g1 r2 g2 r3 g3
        auto rg      = _mm_shufflehi_epi16(rg01_23, 0xD8);  // r0 r1 g0 g1 r2 r3 g2 g3

        auto R = _mm_shuffle_epi32(rg, 0x88);  // r0 r1 r2 r3 r0 r1 r2 r3
        auto G = _mm_shuffle_epi32(rg, 0xDD);  // g0 g1 g2 g3 g0 g1 g2 g3
        *r = sk_unaligned_load<U16>(&R);
        *g = sk_unaligned_load<U16>(&G);
    }