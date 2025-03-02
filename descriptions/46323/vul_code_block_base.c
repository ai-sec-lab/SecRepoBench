if (copy == NULL) {
                xmlFreeNode(ret);
                return(NULL);
            }

            if (insert->last == NULL) {
                insert->children = copy;
            } else {
                copy->prev = insert->last;
                insert->last->next = copy;
            }
            insert->last = copy;