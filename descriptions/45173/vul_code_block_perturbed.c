if (ptr + last > e) {
        gs_throw(-1, "not enough data for index data");
        return 0;
    }

    ptr += last;

    return ptr;