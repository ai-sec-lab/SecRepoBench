static int ositp_decode_CR_CC(tvbuff_t *tvb, int offset, guint8 li, guint8 tpdu,
                              packet_info *pinfo, proto_tree *tree,
                              gboolean uses_inactive_subset,
                              gboolean *subdissector_found)
{
  /* note: in the ATN the user is up to chose between 3 different checksums:
   *       standard OSI, 2 or 4 octet extended checksum.
   * Nothing has to be done here, for all ATN specifics are handled in VP. */

  /* CC & CR decoding in the same function */

  proto_tree *cotp_tree = NULL;
  proto_item *ti;
  proto_item *item = NULL;
  guint16 dst_ref, src_ref;
  guint8  class_option;
  tvbuff_t *next_tvb;
  guint   tpdu_len;
  heur_dtbl_entry_t *hdtbl_entry;
  static const int * class_options[] = {
     &hf_cotp_class,
     &hf_cotp_opts_extended_formats,
     &hf_cotp_opts_no_explicit_flow_control,
     NULL,
  };

  src_ref = tvb_get_ntohs(tvb, offset + P_SRC_REF);

  class_option = tvb_get_guint8(tvb, offset + P_CLASS_OPTION);
  if (((class_option & 0xF0) >> 4) > 4) /* class 0..4 allowed */
    return -1;

  /* CR and CC TPDUs can have user data, so they run to the end of the
   * containing PDU */
  tpdu_len = tvb_reported_length_remaining(tvb, offset);

  dst_ref = tvb_get_ntohs(tvb, offset + P_DST_REF);
  pinfo->clnp_srcref = src_ref;
  pinfo->clnp_dstref = dst_ref;

  col_append_fstr(pinfo->cinfo, COL_INFO,
                  "%s TPDU src-ref: 0x%04x dst-ref: 0x%04x",
                  (tpdu == CR_TPDU) ? "CR" : "CC", src_ref, dst_ref);

  ti = proto_tree_add_item(tree, proto_cotp, tvb, offset, li + 1, ENC_NA);
  cotp_tree = proto_item_add_subtree(ti, ett_cotp);
  proto_tree_add_uint(cotp_tree, hf_cotp_li, tvb, offset, 1,li);
  offset += 1;

  item = proto_tree_add_uint(cotp_tree, hf_cotp_type, tvb, offset, 1, tpdu);
  offset += 1;
  li -= 1;

  proto_tree_add_uint(cotp_tree, hf_cotp_destref, tvb, offset, 2, dst_ref);
  offset += 2;
  li -= 2;

  proto_tree_add_uint(cotp_tree, hf_cotp_srcref, tvb, offset, 2, src_ref);
  offset += 2;
  li -= 2;

  /* expert info, but only if not encapsulated in TCP/SMB */
  /* XXX - the best way to detect seems to be if we have a port set */
  if (pinfo->destport == 0) {
    expert_add_info_format(pinfo, item, &ei_cotp_connection, "Connection %s: 0x%x -> 0x%x", tpdu == CR_TPDU ? "Request(CR)" : "Confirm(CC)", src_ref, dst_ref);
  }

  proto_tree_add_bitmask_list(cotp_tree, tvb, offset, 1, class_options, ENC_NA);
  offset += 1;
  li -= 1;

  if (li > 0) {
    /* There's more data left, so we have the variable part.

       Microsoft's RDP hijacks the variable part of CR and CC PDUs
       for their own user data (RDP runs atop Class 0, which doesn't
       support user data).

       Try what heuristic dissectors we have. */
    next_tvb = tvb_new_subset_length(tvb, offset, li);
    if (dissector_try_heuristic((tpdu == CR_TPDU) ?
                                 cotp_cr_heur_subdissector_list :
                                 cotp_cc_heur_subdissector_list,
                                next_tvb, pinfo, tree, &hdtbl_entry, NULL)) {
      /* A subdissector claimed this, so it really belongs to them. */
      *subdissector_found = TRUE;
    } else {
      /* No heuristic dissector claimed it, so dissect it as a regular
         variable part. */
      ositp_decode_var_part(tvb, offset, li, class_option, tpdu_len, pinfo,
                            cotp_tree);
    }
    offset += li;
  }

  /*
   * XXX - tell the subdissector that this is user data in a CR or
   * CC packet rather than a DT packet?
   */
  // <MASK>

}