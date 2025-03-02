(prefix == NULL) {
                    /*
                     * name is not necessarily null-terminated.
                     */
                    if ((memcmp(entry->name, name, len) == 0) &&
                        (entry->name[len] == 0)) {
                        found = 1;
                        break;
                    }
                } else {
                    if (xmlStrQEqual(prefix, name, entry->name)) {
                        found = 1;
                        break;
                    }
                }