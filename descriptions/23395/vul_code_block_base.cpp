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
    const int colorNameSize = sizeof(gColorNames) / sizeof(unsigned int);
    int lo = 0;
    int hi = colorNameSize - 3; // back off to beginning of yellowgreen
    while (lo <= hi) {
        int mid = (hi + lo) >> 1;
        while ((int) gColorNames[mid] >= 0)
            --mid;
        sixMatchPtr = sixMatches;
        while (gColorNames[mid] == *sixMatchPtr) {
            ++mid;
            if ((*sixMatchPtr & 1) == 0) { // last
                *color = gColorNames[mid] | 0xFF000000;
                return namePtr;
            }
            ++sixMatchPtr;
        }
        int sixMask = *sixMatchPtr & ~0x80000000;
        int midMask = gColorNames[mid] & ~0x80000000;
        if (sixMask > midMask) {
            lo = mid + 2;   // skip color
            while ((int) gColorNames[lo] >= 0)
                ++lo;
        } else if (hi == mid)
            return nullptr;
        else
            hi = mid;
    }