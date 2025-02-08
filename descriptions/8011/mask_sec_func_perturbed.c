int
dissect_ber_constrained_bitstring(gboolean implicit_tag, asn1_ctx_t *actx, proto_tree *parent_tree, tvbuff_t *tvb, int offset, gint32 min_len, gint32 maxlength, const asn_namedbit *named_bits, gint hf_id, gint ett_id, tvbuff_t **out_tvb)
{
    gint8       ber_class;
    gboolean    pc, ind;
    gint32      tag;
    guint32     len, byteno;
    guint8      pad  = 0, b0, b1, val, *bitstring;
    int         end_offset;
    int         hoffset;
    proto_item *item = NULL;
    proto_item *cause;
    proto_tree *tree = NULL;
    const char *sep;
    gboolean    term;
    const asn_namedbit *nb;

    if (!implicit_tag) {
        hoffset = offset;
        /* read header and len for the octet string */
        offset = dissect_ber_identifier(actx->pinfo, parent_tree, tvb, offset, &ber_class, &pc, &tag);
        offset = dissect_ber_length(actx->pinfo, parent_tree, tvb, offset, &len, &ind);
        end_offset = offset + len;

        /* sanity check: we only handle Universal BitStrings */

        /* for an IMPLICIT APPLICATION tag asn2eth seems to call this
           function with implicit_tag = FALSE. BER_FLAGS_NOOWNTAG was
           set so the APPLICATION tag was still present.
           So here we relax it for APPLICATION tags. CONTEXT tags may
           still cause a problem. */

        if (!implicit_tag && (ber_class != BER_CLASS_APP)) {
            if ( (ber_class != BER_CLASS_UNI)
              || (tag != BER_UNI_TAG_BITSTRING) ) {
                tvb_ensure_bytes_exist(tvb, hoffset, 2);
                cause = proto_tree_add_string_format_value(
                    parent_tree, hf_ber_error, tvb, offset, len, "bitstring_expected",
                    "BitString expected but class:%s(%d) %s tag:%d was unexpected",
                    val_to_str_const(ber_class, ber_class_codes, "Unknown"),
                    ber_class, pc ? ber_pc_codes_short.true_string : ber_pc_codes_short.false_string,
                    tag);
                expert_add_info(actx->pinfo, cause, &ei_ber_expected_bitstring);
                if (decode_unexpected) {
                    proto_tree *unknown_tree = proto_item_add_subtree(cause, ett_ber_unknown);
                    dissect_unknown_ber(actx->pinfo, tvb, hoffset, unknown_tree);
                }
                return end_offset;
            }
        }
    } else {
        pc=0;
        len = tvb_reported_length_remaining(tvb, offset);
        end_offset = offset+len;
    }
    // <MASK>
}