int av_aes_init(AVAES *a, const uint8_t *key, int key_bits, int decrypt)
{
    int i, j, t, rconpointer = 0;
    uint8_t tk[8][4];
    int KC = key_bits >> 5;
    int rounds = KC + 6;
    uint8_t log8[256];
    uint8_t alog8[512];

    a->crypt = decrypt ? aes_decrypt : aes_encrypt;

    if (!enc_multbl[FF_ARRAY_ELEMS(enc_multbl) - 1][FF_ARRAY_ELEMS(enc_multbl[0]) - 1]) {
        j = 1;
        for (i = 0; i < 255; i++) {
            alog8[i] = alog8[i + 255] = j;
            log8[j] = i;
            j ^= j + j;
            if (j > 255)
                j ^= 0x11B;
        }
        for (i = 0; i < 256; i++) {
            j = i ? alog8[255 - log8[i]] : 0;
            j ^= (j << 1) ^ (j << 2) ^ (j << 3) ^ (j << 4);
            j = (j ^ (j >> 8) ^ 99) & 255;
            inv_sbox[j] = i;
            sbox[i]     = j;
        }
        init_multbl2(dec_multbl, (const int[4]) { 0xe, 0x9, 0xd, 0xb },
                     log8, alog8, inv_sbox);
        init_multbl2(enc_multbl, (const int[4]) { 0x2, 0x1, 0x1, 0x3 },
                     log8, alog8, sbox);
    }

    if (key_bits != 128 && key_bits != 192 && key_bits != 256)
        return AVERROR(EINVAL);

    a->rounds = rounds;

    memcpy(tk, key, KC * 4);
    memcpy(a->round_key[0].u8, key, KC * 4);

    // <MASK>

    return 0;
}