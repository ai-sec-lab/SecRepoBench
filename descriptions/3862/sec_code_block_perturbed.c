if (available_space(pe, names + index) < sizeof(DWORD) ||
        available_space(pe, ordinals + index) < sizeof(WORD))
    {
      break;
    }

    offset = pe_rva_to_offset(pe, names[index]);

    if (offset < 0)
      continue;

    remaining = pe->data_size - (size_t) offset;
    name = yr_strndup((char*) (pe->data + offset), remaining);

    // Get the corresponding ordinal. Note that we are not subtracting the
    // ordinal base here as we don't intend to index into the export address
    // table.
    ordinal = yr_le16toh(ordinals[index]);

    // Now add it to the list...
    exported_func = (IMPORT_EXPORT_FUNCTION*)
        yr_calloc(1, sizeof(IMPORT_EXPORT_FUNCTION));

    if (exported_func == NULL)
    {
      yr_free(name);
      continue;
    }

    exported_func->name = name;
    exported_func->ordinal = ordinal;
    exported_func->next = NULL;

    if (head == NULL)
      head = exported_func;

    if (tail != NULL)
      tail->next = exported_func;

    tail = exported_func;
    num_exports++;