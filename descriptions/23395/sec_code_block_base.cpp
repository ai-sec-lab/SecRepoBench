do {
        unsigned int sixMatch = 0;
        for (int chIndex = 0; chIndex < 6; chIndex++) {
            sixMatch <<= 5;
            ch = *namePtr  | 0x20;
            if (ch < 'a' || ch > 'z')
                ch = 0;
            else {
                ch = ch - 'a' + 1;
                namePtr++;
            }
            sixMatch |= ch ;  // turn 'A' (0x41) into 'a' (0x61);
        }
        sixMatch <<= 1;
        sixMatch |= 1;
        if (first) {
            sixMatch |= 0x80000000;
            first = false;
        }
        ch = *namePtr | 0x20;
        last = ch < 'a' || ch > 'z';
        if (last)
            sixMatch &= ~1;
        len -= 6;
        *sixMatchPtr++ = sixMatch;
    } while (last == false && len > 0);

    auto is_rec_start = [](size_t i) {
        SkASSERT(i < SK_ARRAY_COUNT(gColorNames));

        // the first record word is tagged in bit 31
        return static_cast<int>(gColorNames[i]) < 0;
    };

    auto rec_start = [&](size_t i, int dir) {
        // find the beginning of a record, searching in the given direction.
        while (!is_rec_start(i)) {
            i += dir;
        }

        return i;
    };

    size_t lo = 0,
           hi = SK_ARRAY_COUNT(gColorNames) - 3; // back off to beginning of yellowgreen

    while (lo <= hi) {
        SkASSERT(is_rec_start(lo));
        SkASSERT(is_rec_start(hi));

        const auto  mid         = rec_start((hi + lo) >> 1, -1);
        const auto* midMatchPtr = &gColorNames[mid]; // separate cursor, to keep |mid| on rec start

        sixMatchPtr = sixMatches;
        while (*midMatchPtr == *sixMatchPtr) {
            ++midMatchPtr;
            if ((*sixMatchPtr & 1) == 0) { // last
                // jackpot
                *color = *midMatchPtr | 0xFF000000;
                return namePtr;
            }
            ++sixMatchPtr;
        }

        if (hi == lo) {
            // not found
            break;
        }

        int sixMask = *sixMatchPtr & ~0x80000000,
            midMask = *midMatchPtr & ~0x80000000;

        SkASSERT(sixMask != midMask);
        if (sixMask > midMask) {
            // skip color
            lo = rec_start(mid + 2,  1);
        } else {
            hi = mid;
        }
    }