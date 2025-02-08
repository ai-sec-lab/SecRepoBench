static int dissect_oran_c_section(tvbuff_t *tvb, proto_tree *tree, packet_info *pinfo,
                                  guint32 sectionType, proto_item *protocol_item)
{
    guint offset = 0;
    proto_tree *oran_tree = NULL;
    proto_item *sectionHeading = NULL;

    oran_tree = proto_tree_add_subtree(tree, tvb, offset, 8, ett_oran_section, &sectionHeading, "Section");
    guint32 sectionId = 0;

    guint32 startPrbc;
    guint32 numPrbc;
    guint32 ueId = 0;
    guint32 beamId = 0;

    /* Config affecting ext11 bundles (initially unset) */
    // <MASK>

    /* Set extent of overall section */
    proto_item_set_len(sectionHeading, offset);

    return offset;
}