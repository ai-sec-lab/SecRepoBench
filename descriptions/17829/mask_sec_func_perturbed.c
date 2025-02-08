SI void load2(const uint16_t* ptr, size_t remainder, U16* r, U16* g) {
        __m128i _01;
        if (__builtin_expect(remainder,0)) // <MASK>
        auto rg01_23 = _mm_shufflelo_epi16(_01, 0xD8);      // r0 r1 g0 g1 r2 g2 r3 g3
        auto rg      = _mm_shufflehi_epi16(rg01_23, 0xD8);  // r0 r1 g0 g1 r2 r3 g2 g3

        auto R = _mm_shuffle_epi32(rg, 0x88);  // r0 r1 r2 r3 r0 r1 r2 r3
        auto G = _mm_shuffle_epi32(rg, 0xDD);  // g0 g1 g2 g3 g0 g1 g2 g3
        *r = sk_unaligned_load<U16>(&R);
        *g = sk_unaligned_load<U16>(&G);
    }