if (archive->cstate.window_size)
        archive->cstate.window_mask = archive->cstate.window_size - 1;
    else
        archive->cstate.window_mask = 0;

    if(archive->cstate.window_buf)
        free(archive->cstate.window_buf);

    if(archive->cstate.filtered_buf)
        free(archive->cstate.filtered_buf);

    archive->cstate.window_buf = calloc(1, archive->cstate.window_size);
    archive->cstate.filtered_buf = calloc(1, archive->cstate.window_size);

    archive->cstate.write_ptr = 0;
    archive->cstate.last_write_ptr = 0;

    memset(&archive->cstate.bd, 0, sizeof(archive->cstate.bd));
    memset(&archive->cstate.ld, 0, sizeof(archive->cstate.ld));
    memset(&archive->cstate.dd, 0, sizeof(archive->cstate.dd));
    memset(&archive->cstate.ldd, 0, sizeof(archive->cstate.ldd));
    memset(&archive->cstate.rd, 0, sizeof(archive->cstate.rd));