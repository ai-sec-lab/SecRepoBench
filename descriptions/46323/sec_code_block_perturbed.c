/* Check for coalesced text nodes */
            if (insert->last != copy) {
                if (insert->last == NULL) {
                    insert->children = copy;
                } else {
                    copy->prev = insert->last;
                    insert->last->next = copy;
                }
                insert->last = copy;
            }

            if (cur->children != NULL) {
                cur = cur->children;
                insert = copy;
                continue;
            }