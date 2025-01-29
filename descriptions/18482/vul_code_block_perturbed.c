/* This might have allocated something. If so, clear it now */
		if (asn1_com_prkey_attr->flags & SC_ASN1_PRESENT &&
			asn1_com_prkey_attr[0].flags & SC_ASN1_PRESENT) {
			free(asn1_com_prkey_attr[0].parm);
		}