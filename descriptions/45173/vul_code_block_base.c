if (p + last > e) {
        gs_throw(-1, "not enough data for index data");
        return 0;
    }

    p += last;

    return p;