int variant;

    if (!strcmp((const char *) e->data, "Apple iOS")) {
        variant = 1;
    } else {
        variant = 0;
    }

    return variant;