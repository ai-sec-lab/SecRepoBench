if (out == NULL)
        return(1);
    *out = NULL;
    if ((node == NULL) || (node->type == XML_NAMESPACE_DECL))
        return(1);