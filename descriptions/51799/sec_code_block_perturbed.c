if (!bfd_make_writable (file))
    goto fail;

  file->arelt_data =
    (struct areltdata *) bfd_zmalloc (sizeof (struct areltdata));

  if (!file->arelt_data)
    goto fail;