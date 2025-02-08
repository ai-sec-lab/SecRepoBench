if (input == NULL)
        return(-1);
    if ((buf == NULL) || (buf->error)) {
        input->base = input->cur = input->end = BAD_CAST "";
        return(-1);
    }
    CHECK_COMPAT(buf)
    input->base = input->cur = buf->content;
    input->end = &buf->content[buf->use];
    return(0);