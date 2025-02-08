if (idx >= nb_vectors)
                return AVERROR_INVALIDDATA;

            dsty[x  +frame->linesize[0]] = vec[idx * 12 + 0];
            dsty[x+1+frame->linesize[0]] = vec[idx * 12 + 3];
            dsty[x]                      = vec[idx * 12 + 6];
            dsty[x+1]                    = vec[idx * 12 + 9];

            dstu[x  +frame->linesize[1]] = vec[idx * 12 + 1];
            dstu[x+1+frame->linesize[1]] = vec[idx * 12 + 4];
            dstu[x]                      = vec[idx * 12 + 7];
            dstu[x+1]                    = vec[idx * 12 +10];

            dstv[x  +frame->linesize[2]] = vec[idx * 12 + 2];
            dstv[x+1+frame->linesize[2]] = vec[idx * 12 + 5];
            dstv[x]                      = vec[idx * 12 + 8];
            dstv[x+1]                    = vec[idx * 12 +11];