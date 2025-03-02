if (copy == NULL) {
                xmlFreeNode(ret);
                return(NULL);
            }

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