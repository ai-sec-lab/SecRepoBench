if (cur == (xmlNodePtr) sourceDoc)
                return;
            while (cur->next == NULL) {
                cur = cur->parent;
                if (cur == (xmlNodePtr) sourceDoc)
                    return;
            }

            cur = cur->next;