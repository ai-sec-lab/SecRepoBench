/*
                     * name is not necessarily null-terminated.
                     */
                    if ((strncmp((const char *) entry->name,
                                 (const char *) name, len) == 0) &&
                        (entry->name[len] == 0)) {
                        found = 1;
                        break;
                    }