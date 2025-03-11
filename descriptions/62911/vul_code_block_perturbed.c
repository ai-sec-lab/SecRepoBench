(prefix == NULL) {
                    /*
                     * name is not necessarily null-terminated.
                     */
                    if ((memcmp(entry->name, key_name, len) == 0) &&
                        (entry->name[len] == 0)) {
                        found = 1;
                        break;
                    }
                } else {
                    if (xmlStrQEqual(prefix, key_name, entry->name)) {
                        found = 1;
                        break;
                    }
                }