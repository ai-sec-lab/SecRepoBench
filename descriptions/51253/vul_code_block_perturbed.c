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

        memcpy(a->round_key[0].u8 + t, tk, KC * 4);
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