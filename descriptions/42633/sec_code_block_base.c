int s1 = s->cur[-((si * 2) + 1)].size < MAX_CMAP_CODE_SIZE ? s->cur[-((si * 2) + 1)].size : MAX_CMAP_CODE_SIZE;
            int s2 = s->cur[-(si * 2)].size < MAX_CMAP_CODE_SIZE ? s->cur[-(si * 2)].size : MAX_CMAP_CODE_SIZE;

            memcpy(code_space->ranges[i].first, s->cur[-((si * 2) + 1)].val.string, s1);
            memcpy(code_space->ranges[i].last, s->cur[-(si * 2)].val.string, s2);