if ((input == NULL) || (buf == NULL) || (buf->error))
        return(-1);
    CHECK_COMPAT(buf)
    input->base = input->cur = buf->content;
    input->end = &buf->content[buf->use];
    return(0);