val = atoi(time);

    /* String time to seconds */
    if (time[len - 1] == 'D' || time[len - 1] == 'd') {
        val *= 86400;
    }
    if (time[len - 1] == 'H' || time[len - 1] == 'h') {
        val *= 3600;
    }
    else if (time[len - 1] == 'M' || time[len - 1] == 'm') {
        val *= 60;
    }

    return val;