void
proto_register_tls(void)
{

    /* Setup list of header fields See Section 1.6.1 for details*/
    static hf_register_info hf[] = {
        { &hf_tls_record,
          { "Record Layer", "tls.record",
            FT_NONE, BASE_NONE, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_tls_record_content_type,
          { "Content Type", "tls.record.content_type",
            FT_UINT8, BASE_DEC, VALS(ssl_31_content_type), 0x0,
            NULL, HFILL}
        },
        { &hf_tls_record_opaque_type,
          { "Opaque Type", "tls.record.opaque_type",
            FT_UINT8, BASE_DEC, VALS(ssl_31_content_type), 0x0,
            "Always set to value 23, actual content type is known after decryption", HFILL}
        },
        { &hf_ssl2_msg_type,
          { "Handshake Message Type", "tls.handshake.type",
            FT_UINT8, BASE_DEC, VALS(ssl_20_msg_types), 0x0,
            "SSLv2 handshake message type", HFILL}
        },
        { &hf_tls_record_version,
          { "Version", "tls.record.version",
            FT_UINT16, BASE_HEX, VALS(ssl_versions), 0x0,
            "Record layer version", HFILL }
        },
        { &hf_tls_record_length,
          { "Length", "tls.record.length",
            FT_UINT16, BASE_DEC, NULL, 0x0,
            "Length of TLS record data", HFILL }
        },
        { &hf_tls_record_appdata,
          { "Encrypted Application Data", "tls.app_data",
            FT_BYTES, BASE_NONE, NULL, 0x0,
            "Payload is encrypted application data", HFILL }
        },

        { &hf_ssl2_record,
          { "SSLv2 Record Header", "tls.record",
            FT_NONE, BASE_NONE, NULL, 0x0,
            "SSLv2 record data", HFILL }
        },
        { &hf_ssl2_record_is_escape,
          { "Is Escape", "tls.record.is_escape",
            FT_BOOLEAN, BASE_NONE, NULL, 0x0,
            "Indicates a security escape", HFILL}
        },
        { &hf_ssl2_record_padding_length,
          { "Padding Length", "tls.record.padding_length",
            FT_UINT8, BASE_DEC, NULL, 0x0,
            "Length of padding at end of record", HFILL }
        },
        { &hf_tls_alert_message,
          { "Alert Message", "tls.alert_message",
            FT_NONE, BASE_NONE, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_tls_alert_message_level,
          { "Level", "tls.alert_message.level",
            FT_UINT8, BASE_DEC, VALS(ssl_31_alert_level), 0x0,
            "Alert message level", HFILL }
        },
        { &hf_tls_alert_message_description,
          { "Description", "tls.alert_message.desc",
            FT_UINT8, BASE_DEC, VALS(ssl_31_alert_description), 0x0,
            "Alert message description", HFILL }
        },
        { &hf_tls_handshake_protocol,
          { "Handshake Protocol", "tls.handshake",
            FT_NONE, BASE_NONE, NULL, 0x0,
            "Handshake protocol message", HFILL}
        },
        { &hf_tls_handshake_type,
          { "Handshake Type", "tls.handshake.type",
            FT_UINT8, BASE_DEC, VALS(ssl_31_handshake_type), 0x0,
            "Type of handshake message", HFILL}
        },
        { &hf_tls_handshake_length,
          { "Length", "tls.handshake.length",
            FT_UINT24, BASE_DEC, NULL, 0x0,
            "Length of handshake message", HFILL }
        },
        { &hf_ssl2_handshake_cipher_spec,
          { "Cipher Spec", "tls.handshake.cipherspec",
            FT_UINT24, BASE_HEX|BASE_EXT_STRING, &ssl_20_cipher_suites_ext, 0x0,
            "Cipher specification", HFILL }
        },
        { &hf_tls_handshake_npn_selected_protocol_len,
          { "Selected Protocol Length", "tls.handshake.npn_selected_protocol_len",
            FT_UINT8, BASE_DEC, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_tls_handshake_npn_selected_protocol,
          { "Selected Protocol", "tls.handshake.npn_selected_protocol",
            FT_STRING, BASE_NONE, NULL, 0x0,
            "Protocol to be used for connection", HFILL }
        },
        { &hf_tls_handshake_npn_padding_len,
          { "Padding Length", "tls.handshake.npn_padding_len",
            FT_UINT8, BASE_DEC, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_tls_handshake_npn_padding,
          { "Padding", "tls.handshake.npn_padding",
            FT_BYTES, BASE_NONE, NULL, 0x0,
            NULL, HFILL }
        },
        { &ssl_hfs.hs_md5_hash,
          { "MD5 Hash", "tls.handshake.md5_hash",
            FT_NONE, BASE_NONE, NULL, 0x0,
            "Hash of messages, master_secret, etc.", HFILL }
        },
        { &ssl_hfs.hs_sha_hash,
          { "SHA-1 Hash", "tls.handshake.sha_hash",
            FT_NONE, BASE_NONE, NULL, 0x0,
            "Hash of messages, master_secret, etc.", HFILL }
        },
        { &hf_tls_heartbeat_message,
          { "Heartbeat Message", "tls.heartbeat_message",
            FT_NONE, BASE_NONE, NULL, 0x0,
            NULL, HFILL }
        },
        { &hf_tls_heartbeat_message_type,
          { "Type", "tls.heartbeat_message.type",
            FT_UINT8, BASE_DEC, VALS(tls_heartbeat_type), 0x0,
            "Heartbeat message type", HFILL }
        },
        { &hf_tls_heartbeat_message_payload_length,
          { "Payload Length", "tls.heartbeat_message.payload_length",
            FT_UINT16, BASE_DEC, NULL, 0x00, NULL, HFILL }
        },
        { &hf_tls_heartbeat_message_payload,
          { "Payload Length", "tls.heartbeat_message.payload",
            FT_BYTES, BASE_NONE, NULL, 0x00, NULL, HFILL }
        },
        { &hf_tls_heartbeat_message_padding,
          { "Payload Length", "tls.heartbeat_message.padding",
            FT_BYTES, BASE_NONE, NULL, 0x00, NULL, HFILL }
        },
        { &hf_ssl2_handshake_challenge,
          { "Challenge", "tls.handshake.challenge",
            FT_NONE, BASE_NONE, NULL, 0x0,
            "Challenge data used to authenticate server", HFILL }
        },
        { &hf_ssl2_handshake_cipher_spec_len,
          { "Cipher Spec Length", "tls.handshake.cipher_spec_len",
            FT_UINT16, BASE_DEC, NULL, 0x0,
            "Length of cipher specs field", HFILL }
        },
        { &hf_ssl2_handshake_session_id_len,
          { "Session ID Length", "tls.handshake.session_id_length",
            FT_UINT16, BASE_DEC, NULL, 0x0,
            "Length of session ID field", HFILL }
        },
        { &hf_ssl2_handshake_challenge_len,
          { "Challenge Length", "tls.handshake.challenge_length",
            FT_UINT16, BASE_DEC, NULL, 0x0,
            "Length of challenge field", HFILL }
        },
        { &hf_ssl2_handshake_clear_key_len,
          { "Clear Key Data Length", "tls.handshake.clear_key_length",
            FT_UINT16, BASE_DEC, NULL, 0x0,
            "Length of clear key data", HFILL }
        },
        { &hf_ssl2_handshake_enc_key_len,
          { "Encrypted Key Data Length", "tls.handshake.encrypted_key_length",
            FT_UINT16, BASE_DEC, NULL, 0x0,
            "Length of encrypted key data", HFILL }
        },
        { &hf_ssl2_handshake_key_arg_len,
          { "Key Argument Length", "tls.handshake.key_arg_length",
            FT_UINT16, BASE_DEC, NULL, 0x0,
            "Length of key argument", HFILL }
        },
        { &hf_ssl2_handshake_clear_key,
          { "Clear Key Data", "tls.handshake.clear_key_data",
            FT_NONE, BASE_NONE, NULL, 0x0,
            "Clear portion of MASTER-KEY", HFILL }
        },
        { &hf_ssl2_handshake_enc_key,
          { "Encrypted Key", "tls.handshake.encrypted_key",
            FT_NONE, BASE_NONE, NULL, 0x0,
            "Secret portion of MASTER-KEY encrypted to server", HFILL }
        },
        { &hf_ssl2_handshake_key_arg,
          { "Key Argument", "tls.handshake.key_arg",
            FT_NONE, BASE_NONE, NULL, 0x0,
            "Key Argument (e.g., Initialization Vector)", HFILL }
        },
        { &hf_ssl2_handshake_session_id_hit,
          { "Session ID Hit", "tls.handshake.session_id_hit",
            FT_BOOLEAN, BASE_NONE, NULL, 0x0,
            "Did the server find the client's Session ID?", HFILL }
        },
        { &hf_ssl2_handshake_cert_type,
          { "Certificate Type", "tls.handshake.cert_type",
            FT_UINT8, BASE_DEC, VALS(ssl_20_certificate_type), 0x0,
            NULL, HFILL }
        },
        { &hf_ssl2_handshake_connection_id_len,
          { "Connection ID Length", "tls.handshake.connection_id_length",
            FT_UINT16, BASE_DEC, NULL, 0x0,
            "Length of connection ID", HFILL }
        },
        { &hf_ssl2_handshake_connection_id,
          { "Connection ID", "tls.handshake.connection_id",
            FT_NONE, BASE_NONE, NULL, 0x0,
            "Server's challenge to client", HFILL }
        },

        { &hf_tls_segment_overlap,
          { "Segment overlap", "tls.segment.overlap",
            FT_BOOLEAN, BASE_NONE, NULL, 0x0,
            "Segment overlaps with other segments", HFILL }},

        { &hf_tls_segment_overlap_conflict,
          { "Conflicting data in segment overlap", "tls.segment.overlap.conflict",
            FT_BOOLEAN, BASE_NONE, NULL, 0x0,
            "Overlapping segments contained conflicting data", HFILL }},

        { &hf_tls_segment_multiple_tails,
          { "Multiple tail segments found", "tls.segment.multipletails",
            FT_BOOLEAN, BASE_NONE, NULL, 0x0,
            "Several tails were found when reassembling the pdu", HFILL }},

        { &hf_tls_segment_too_long_fragment,
          { "Segment too long", "tls.segment.toolongfragment",
            FT_BOOLEAN, BASE_NONE, NULL, 0x0,
            "Segment contained data past end of the pdu", HFILL }},

        { &hf_tls_segment_error,
          { "Reassembling error", "tls.segment.error",
            FT_FRAMENUM, BASE_NONE, NULL, 0x0,
            "Reassembling error due to illegal segments", HFILL }},

        { &hf_tls_segment_count,
          { "Segment count", "tls.segment.count",
            FT_UINT32, BASE_DEC, NULL, 0x0,
            NULL, HFILL }},

        { &hf_tls_segment,
          { "TLS segment", "tls.segment",
            FT_FRAMENUM, BASE_NONE, NULL, 0x0,
            NULL, HFILL }},

        { &hf_tls_segments,
          { "Reassembled TLS segments", "tls.segments",
            FT_NONE, BASE_NONE, NULL, 0x0,
            "TLS Segments", HFILL }},

        { &hf_tls_reassembled_in,
          { "Reassembled PDU in frame", "tls.reassembled_in",
            FT_FRAMENUM, BASE_NONE, NULL, 0x0,
            "The PDU that doesn't end in this segment is reassembled in this frame", HFILL }},

        { &hf_tls_reassembled_length,
          { "Reassembled PDU length", "tls.reassembled.length",
            FT_UINT32, BASE_DEC, NULL, 0x0,
            "The total length of the reassembled payload", HFILL }},

        { &hf_tls_reassembled_data,
          { "Reassembled PDU data", "tls.reassembled.data",
            FT_BYTES, BASE_NONE, NULL, 0x00,
            "The payload of multiple reassembled TLS segments", HFILL }},

        { &hf_tls_segment_data,
          { "TLS segment data", "tls.segment.data",
            FT_BYTES, BASE_NONE, NULL, 0x00,
            "The payload of a single TLS segment", HFILL }
        },

        { &hf_tls_handshake_fragment_count,
          { "Handshake Fragment count", "tls.handshake.fragment.count",
            FT_UINT32, BASE_DEC, NULL, 0x0,
            NULL, HFILL }},

        { &hf_tls_handshake_fragment,
          { "Handshake Fragment", "tls.handshake.fragment",
            FT_FRAMENUM, BASE_NONE, NULL, 0x0,
            NULL, HFILL }},

        { &hf_tls_handshake_fragments,
          { "Reassembled Handshake Fragments", "tls.handshake.fragments",
            FT_NONE, BASE_NONE, NULL, 0x0,
            NULL, HFILL }},

        { &hf_tls_handshake_reassembled_in,
          { "Reassembled Handshake Message in frame", "tls.handshake.reassembled_in",
            FT_FRAMENUM, BASE_NONE, NULL, 0x0,
            "The handshake message is fully reassembled in this frame", HFILL }},

        SSL_COMMON_HF_LIST(dissect_ssl3_hf, "tls")
    };

    /* Setup protocol subtree array */
    static gint *ett[] = {
        &ett_tls,
        &ett_tls_record,
        &ett_tls_alert,
        &ett_tls_handshake,
        &ett_tls_heartbeat,
        &ett_tls_certs,
        &ett_tls_segments,
        &ett_tls_segment,
        &ett_tls_hs_fragments,
        &ett_tls_hs_fragment,
        SSL_COMMON_ETT_LIST(dissect_ssl3_hf)
    };

    static ei_register_info ei[] = {
        { &ei_ssl2_handshake_session_id_len_error, { "tls.handshake.session_id_length.error", PI_MALFORMED, PI_ERROR, "Session ID length error", EXPFILL }},
        { &ei_ssl3_heartbeat_payload_length, { "tls.heartbeat_message.payload_length.invalid", PI_MALFORMED, PI_ERROR, "Invalid heartbeat payload length", EXPFILL }},
        { &ei_tls_unexpected_message, { "tls.unexpected_message", PI_PROTOCOL, PI_ERROR, "Unexpected message", EXPFILL }},

      /* Generated from convert_proto_tree_add_text.pl */
      { &ei_tls_ignored_unknown_record, { "tls.ignored_unknown_record", PI_PROTOCOL, PI_WARN, "Ignored Unknown Record", EXPFILL }},

        SSL_COMMON_EI_LIST(dissect_ssl3_hf, "tls")
    };

    static build_valid_func ssl_da_src_values[1] = {ssl_src_value};
    static build_valid_func ssl_da_dst_values[1] = {ssl_dst_value};
    static build_valid_func ssl_da_both_values[2] = {ssl_src_value, ssl_dst_value};
    static decode_as_value_t ssl_da_values[3] = {{ssl_src_prompt, 1, ssl_da_src_values}, {ssl_dst_prompt, 1, ssl_da_dst_values}, {ssl_both_prompt, 2, ssl_da_both_values}};
    static decode_as_t tlsda = {"tls", "tls.port", 3, 2, ssl_da_values, "TCP", "port(s) as",
                                 decode_as_default_populate_list, decode_as_default_reset, decode_as_default_change, NULL};

    expert_module_t* expert_ssl;

    /* Register the protocol name and description */
    proto_tls = proto_register_protocol("Transport Layer Security",
                                        "TLS", "tls");

    ssl_associations = register_dissector_table("tls.port", "TLS Port", proto_tls, FT_UINT16, BASE_DEC);
    register_dissector_table_alias(ssl_associations, "ssl.port");

    /* Required function calls to register the header fields and
     * subtrees used */
    proto_register_field_array(proto_tls, hf, array_length(hf));
    proto_register_alias(proto_tls, "ssl");
    proto_register_subtree_array(ett, array_length(ett));
    expert_ssl = expert_register_protocol(proto_tls);
    expert_register_field_array(expert_ssl, ei, array_length(ei));

    {
        module_t *ssl_module = prefs_register_protocol(proto_tls, proto_reg_handoff_ssl);

#ifdef HAVE_LIBGNUTLS
        static uat_field_t sslkeylist_uats_flds[] = {
            UAT_FLD_CSTRING_OTHER(sslkeylist_uats, ipaddr, "IP address", ssldecrypt_uat_fld_ip_chk_cb, "IPv4 or IPv6 address (unused)"),
            UAT_FLD_CSTRING_OTHER(sslkeylist_uats, port, "Port", ssldecrypt_uat_fld_port_chk_cb, "Port Number (optional)"),
            UAT_FLD_CSTRING_OTHER(sslkeylist_uats, protocol, "Protocol", ssldecrypt_uat_fld_protocol_chk_cb, "Application Layer Protocol (optional)"),
            UAT_FLD_FILENAME_OTHER(sslkeylist_uats, keyfile, "Key File", ssldecrypt_uat_fld_fileopen_chk_cb, "Private keyfile."),
            UAT_FLD_CSTRING_OTHER(sslkeylist_uats, password,"Password", ssldecrypt_uat_fld_password_chk_cb, "Password (for PCKS#12 keyfile)"),
            UAT_END_FIELDS
        };

        ssldecrypt_uat = uat_new("TLS Decrypt",
            sizeof(ssldecrypt_assoc_t),
            "ssl_keys",                     /* filename */
            TRUE,                           /* from_profile */
            &tlskeylist_uats,               /* data_ptr */
            &ntlsdecrypt,                   /* numitems_ptr */
            UAT_AFFECTS_DISSECTION,         /* affects dissection of packets, but not set of named fields */
            NULL,                           /* Help section (currently a wiki page) */
            ssldecrypt_copy_cb,
            NULL,
            ssldecrypt_free_cb,
            ssl_parse_uat,
            ssl_reset_uat,
            sslkeylist_uats_flds);

        prefs_register_uat_preference(ssl_module, "key_table",
            "RSA keys list",
            "A table of RSA keys for TLS decryption",
            ssldecrypt_uat);

        prefs_register_string_preference(ssl_module, "keys_list", "RSA keys list (deprecated)",
             "Semicolon-separated list of private RSA keys used for TLS decryption. "
             "Used by versions of Wireshark prior to 1.6",
             &ssl_keys_list);
#endif  /* HAVE_LIBGNUTLS */

        prefs_register_filename_preference(ssl_module, "debug_file", "TLS debug file",
            "Redirect TLS debug to the file specified. Leave empty to disable debugging "
            "or use \"" SSL_DEBUG_USE_STDERR "\" to redirect output to stderr.",
            &ssl_debug_file_name, TRUE);

        prefs_register_bool_preference(ssl_module,
             "desegment_ssl_records",
             "Reassemble TLS records spanning multiple TCP segments",
             "Whether the TLS dissector should reassemble TLS records spanning multiple TCP segments. "
             "To use this option, you must also enable \"Allow subdissectors to reassemble TCP streams\" in the TCP protocol settings.",
             &tls_desegment);
        prefs_register_bool_preference(ssl_module,
             "desegment_ssl_application_data",
             "Reassemble TLS Application Data spanning multiple TLS records",
             "Whether the TLS dissector should reassemble TLS Application Data spanning multiple TLS records. ",
             &tls_desegment_app_data);
        prefs_register_bool_preference(ssl_module,
             "ignore_ssl_mac_failed",
             "Message Authentication Code (MAC), ignore \"mac failed\"",
             "For troubleshooting ignore the mac check result and decrypt also if the Message Authentication Code (MAC) fails.",
             &tls_ignore_mac_failed);
        ssl_common_register_options(ssl_module, &ssl_options, FALSE);
    }

    /* heuristic dissectors for any premable e.g. CredSSP before RDP */
    ssl_heur_subdissector_list = register_heur_dissector_list("tls", proto_tls);

    // <MASK>
    register_cleanup_routine(ssl_cleanup);
    reassembly_table_register(&ssl_reassembly_table,
                          &addresses_ports_reassembly_table_functions);
    reassembly_table_register(&tls_hs_reassembly_table,
                          &addresses_ports_reassembly_table_functions);
    register_decode_as(&tlsda);

    /* XXX: this seems unused due to new "Follow TLS" method, remove? */
    tls_tap = register_tap("tls");
    ssl_debug_printf("proto_register_ssl: registered tap %s:%d\n",
        "tls", tls_tap);

    register_follow_stream(proto_tls, "tls", tcp_follow_conv_filter, tcp_follow_index_filter, tcp_follow_address_filter,
                            tcp_port_to_display, ssl_follow_tap_listener);
    secrets_register_type(SECRETS_TYPE_TLS, tls_secrets_block_callback);
}