int av_aes_init(AVAES *a, const uint8_t *key, int key_bits, int decrypt)
{
    int i, sidx, t, rconpointer = 0;
    uint8_t tk[8][4];
    int KC = key_bits >> 5;
    int rounds = KC + 6;
    uint8_t log8[256];
    uint8_t alog8[512];

    a->crypt = decrypt ? aes_decrypt : aes_encrypt;

    if (!enc_multbl[FF_ARRAY_ELEMS(enc_multbl) - 1][FF_ARRAY_ELEMS(enc_multbl[0]) - 1]) {
        sidx = 1;
        for (i = 0; i < 255; i++) {
            alog8[i] = alog8[i + 255] = sidx;
            log8[sidx] = i;
            sidx ^= sidx + sidx;
            if (sidx > 255)
                sidx ^= 0x11B;
        }
        for (i = 0; i < 256; i++) {
            sidx = i ? alog8[255 - log8[i]] : 0;
            sidx ^= (sidx << 1) ^ (sidx << 2) ^ (sidx << 3) ^ (sidx << 4);
            sidx = (sidx ^ (sidx >> 8) ^ 99) & 255;
            inv_sbox[sidx] = i;
            sbox[i]     = sidx;
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

    for (t = KC * 4; t < (rounds + 1) * 16; t += KC * 4) {
        for (i = 0; i < 4; i++)
            tk[0][i] ^= sbox[tk[KC - 1][(i + 1) & 3]];
        tk[0][0] ^= rcon[rconpointer++];

        for (sidx = 1; sidx < KC; sidx++) {
            if (KC != 8 || sidx != KC >> 1)
                for (i = 0; i < 4; i++)
                    tk[sidx][i] ^= tk[sidx - 1][i];
            else
                for (i = 0; i < 4; i++)
                    tk[sidx][i] ^= sbox[tk[sidx - 1][i]];
        }

        memcpy((unsigned char*)a->round_key + t, tk, KC * 4);
    }

    if (decrypt) {
        for (i = 1; i < rounds; i++) {
            av_aes_block tmp[3];
            tmp[2] = a->round_key[i];
            subshift(&tmp[1], 0, sbox);
            mix(tmp, dec_multbl, 1, 3);
            a->round_key[i] = tmp[0];
        }
    } else {
        for (i = 0; i < (rounds + 1) >> 1; i++)
            FFSWAP(av_aes_block, a->round_key[i], a->round_key[rounds - i]);
    }

    return 0;
}