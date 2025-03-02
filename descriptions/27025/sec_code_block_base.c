(m->type == FLB_CONFIG_MAP_STR) {
                entry->val.str = flb_sds_create(kv->val);
            }
            else if (m->type == FLB_CONFIG_MAP_INT) {
                entry->val.i_num = atoi(kv->val);
            }
            else if (m->type == FLB_CONFIG_MAP_DOUBLE) {
                entry->val.d_num = atof(kv->val);
            }
            else if (m->type == FLB_CONFIG_MAP_SIZE) {
                entry->val.s_num = flb_utils_size_to_bytes(kv->val);
            }
            else if (m->type == FLB_CONFIG_MAP_TIME) {
                entry->val.i_num = flb_utils_time_to_seconds(kv->val);
            }
            else if (m->type == FLB_CONFIG_MAP_BOOL) {
                ret = flb_utils_bool(kv->val);
                if (ret == -1) {
                    flb_free(entry);
                    flb_error("[config map] invalid value for boolean property '%s=%s'",
                              m->name, kv->val);
                    return -1;
                }
                entry->val.boolean = ret;
            }
            else if (m->type >= FLB_CONFIG_MAP_CLIST ||
                     m->type <= FLB_CONFIG_MAP_SLIST_4) {

                list = parse_string_map_to_list(m, kv->val);
                if (!list) {
                    flb_error("[config map] cannot parse list of values '%s'", kv->val);
                    flb_free(entry);
                    return -1;
                }
                entry->val.list = list;

                /* Validate the number of entries are the minimum expected */
                len = mk_list_size(list);
                ret = check_list_size(list, m->type);
                if (ret == -1) {
                    flb_error("[config map] property '%s' expects %i values "
                              "(only %i were found)",
                              kv->key,
                              flb_config_map_expected_values(m->type), len);
                    /*
                     * Register the entry anyways, so on exit the resources will
                     * be released
                     */
                    mk_list_add(&entry->_head, m->value.mult);
                    return -1;
                }
            }