if (tag)
		acls = sc_asn1_find_tag(ctx, tag, taglen, IASECC_DOCP_TAG_ACLS_CONTACT, &taglen);
	else
		acls = sc_asn1_find_tag(ctx, buf, bufferlength, IASECC_DOCP_TAG_ACLS_CONTACT, &taglen);

	if (!acls || taglen < 7)   {
		sc_log(ctx,
		       "ACLs not found in data(%"SC_FORMAT_LEN_SIZE_T"u) %s",
		       bufferlength, sc_dump_hex(buf, bufferlength));
		LOG_TEST_RET(ctx, SC_ERROR_OBJECT_NOT_FOUND, "ACLs tag missing");
	}

	sc_log(ctx, "ACLs(%"SC_FORMAT_LEN_SIZE_T"u) '%s'", taglen,
	       sc_dump_hex(acls, taglen));
	mask = 0x40, offs = 1;
	for (ii = 0; ii < 7; ii++, mask /= 2)  {
		unsigned char op = file->type == SC_FILE_TYPE_DF ? ops_DF[ii] : ops_EF[ii];

		if (!(mask & acls[0]))
			continue;

		sc_log(ctx, "ACLs mask 0x%X, offs %i, op 0x%X, acls[offs] 0x%X", mask, offs, op, acls[offs]);
		if (op == 0xFF)   {
			;
		}
		else if (acls[offs] == 0)   {
			sc_file_add_acl_entry(file, op, SC_AC_NONE, 0);
		}
		else if (acls[offs] == 0xFF)   {
			sc_file_add_acl_entry(file, op, SC_AC_NEVER, 0);
		}
		else if ((acls[offs] & IASECC_SCB_METHOD_MASK) == IASECC_SCB_METHOD_USER_AUTH)   {
			sc_file_add_acl_entry(file, op, SC_AC_SEN, acls[offs] & IASECC_SCB_METHOD_MASK_REF);
		}
		else if (acls[offs] & IASECC_SCB_METHOD_MASK)   {
			sc_file_add_acl_entry(file, op, SC_AC_SCB, acls[offs]);
		}
		else   {
			sc_log(ctx, "Warning: non supported SCB method: %X", acls[offs]);
			sc_file_add_acl_entry(file, op, SC_AC_NEVER, 0);
		}

		offs++;
	}

	LOG_FUNC_RETURN(ctx, 0);