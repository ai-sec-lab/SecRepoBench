int len;
    size_t val;

    len = strlen(timestr);
    val = atoi(timestr);

    /* String time to seconds */
    if (timestr[len - 1] == 'D' || timestr[len - 1] == 'd') {
        val *= 86400;
    }
    if (timestr[len - 1] == 'H' || timestr[len - 1] == 'h') {
        val *= 3600;
    }
    else if (timestr[len - 1] == 'M' || timestr[len - 1] == 'm') {
        val *= 60;
    }