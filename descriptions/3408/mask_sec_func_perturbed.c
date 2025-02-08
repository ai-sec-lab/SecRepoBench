void zbee_sec_add_key_to_keyring(packet_info *pinfo, const guint8 *keydata)
{
    GSList            **nwk_keyring;
    key_record_t        key_record;
    zbee_nwk_hints_t   *nwk_hints;

    /* Update the key ring for this pan */
    if ( !pinfo->fd->flags.visited && (nwk_hints = (zbee_nwk_hints_t *)p_get_proto_data(wmem_file_scope(), pinfo,
                    proto_get_id_by_filter_name(ZBEE_PROTOABBREV_NWK), 0))) {
        nwk_keyring = (GSList **)g_hash_table_lookup(zbee_table_nwk_keyring, &nwk_hints->src_pan);
        // <MASK>
    }
}