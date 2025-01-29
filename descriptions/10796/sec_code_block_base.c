/* 'actions' may have been reallocated by ofpbuf_put(). */
                nested = ofpbuf_at_assert(actions, start_ofs, sizeof *nested);
                nested->nla_type = OVS_ACTION_ATTR_SET_MASKED;

                key = nested + 1;
                /* Add new padding as needed */
                ofpbuf_put_zeros(actions, NLA_ALIGN(key->nla_len) -
                                          key->nla_len);