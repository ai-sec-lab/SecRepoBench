if (cur == (xmlNodePtr) doc)
                return;
            while (cur->next == NULL) {
                cur = cur->parent;
                if (cur == (xmlNodePtr) doc)
                    return;
            }

            cur = cur->next;