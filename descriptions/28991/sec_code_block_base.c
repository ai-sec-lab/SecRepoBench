for (j = SP_WORD_SIZE - 8; j >= 0; j -= 8) {
                if (((a->dp[i] >> j) & 0xff) != 0) {
                    break;
                }
                else if (j == 0) {
                    j = SP_WORD_SIZE - 8;
                    --i;
                }
            }
            j += 4;
    #else
            for (j = SP_WORD_SIZE - 4; j >= 0; j -= 4) {
                if (((a->dp[i] >> j) & 0xf) != 0) {
                    break;
                }
                else if (j == 0) {
                    j = SP_WORD_SIZE - 4;
                    --i;
                }
            }