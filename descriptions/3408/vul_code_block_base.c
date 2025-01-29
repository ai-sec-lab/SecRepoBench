if ( !nwk_keyring ) {
            nwk_keyring = (GSList **)g_malloc0(sizeof(GSList*));
            g_hash_table_insert(zbee_table_nwk_keyring,
                    g_memdup(&nwk_hints->src_pan, sizeof(nwk_hints->src_pan)), nwk_keyring);
        }

        if ( nwk_keyring ) {
            if ( !*nwk_keyring ||
                    memcmp( ((key_record_t *)((GSList *)(*nwk_keyring))->data)->key, &key,
                        ZBEE_APS_CMD_KEY_LENGTH) ) {
                /* Store a new or different key in the key ring */
                key_record.frame_num = pinfo->num;
                key_record.label = NULL;
                memcpy(&key_record.key, key, ZBEE_APS_CMD_KEY_LENGTH);
                *nwk_keyring = g_slist_prepend(*nwk_keyring, g_memdup(&key_record, sizeof(key_record_t)));
            }
        }