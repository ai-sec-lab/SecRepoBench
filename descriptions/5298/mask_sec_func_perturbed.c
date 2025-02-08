static int
dissect_metric_reporting_policy(tvbuff_t *tvbbuffer, packet_info *pinfo _U_,
        proto_tree *tree, guint offset)
{
    guint8 radio_count = 0, radio_index = 0;
    proto_tree *radio_list = NULL;
    proto_tree *radio_tree = NULL;
    proto_item *pi = NULL;
    guint saved_offset = 0;
    static const int *ieee1905_reporting_policy_flags[] = {
        &hf_ieee1905_assoc_sta_traffic_stats_inclusion,
        &hf_ieee1905_assoc_sta_link_metrics_inclusion,
        &hf_ieee1905_reporting_policy_flags_reserved// <MASK>;

    proto_tree_add_item(tree, hf_ieee1905_ap_metrics_reporting_interval,
                        tvbbuffer, offset, 1, ENC_NA);
    offset++;

    radio_count = tvb_get_guint8(tvbbuffer, offset);
    proto_tree_add_item(tree, hf_ieee1905_metric_reporting_radio_count,
                        tvbbuffer, offset, 1, ENC_NA);
    offset++;

    if (radio_count == 0)
        return offset;

    radio_list = proto_tree_add_subtree(tree, tvbbuffer, offset, -1,
                        ett_metric_reporting_policy_list, &pi,
                        "Metric reporting policy list");
    saved_offset = offset;

    while (radio_index < radio_count) {
        radio_tree = proto_tree_add_subtree_format(radio_list,
                                tvbbuffer, offset, 8,
                                ett_metric_reporting_policy_tree, NULL,
                                "Reporting policy for radio %u",
                                radio_index);

        proto_tree_add_item(radio_tree, hf_ieee1905_metric_reporting_policy_radio_id,
                            tvbbuffer, offset, 6, ENC_NA);
        offset += 6;

        proto_tree_add_item(radio_tree, hf_ieee1905_metrics_rssi_threshold, tvbbuffer,
                            offset, 1, ENC_NA);
        offset++;

        proto_tree_add_item(radio_tree, hf_ieee1905_metric_reporting_rssi_hysteresis,
                            tvbbuffer, offset, 1, ENC_NA);
        offset++;

        proto_tree_add_item(radio_tree, hf_ieee1905_metrics_channel_util_threshold,
                            tvbbuffer, offset, 1, ENC_NA);
        offset++;

        proto_tree_add_bitmask_with_flags(radio_tree, tvbbuffer, offset,
                            hf_ieee1905_metrics_policy_flags,
                            ett_metric_policy_flags,
                            ieee1905_reporting_policy_flags, ENC_NA,
                            BMT_NO_APPEND);
        offset++;

        radio_index++;
    }

    proto_item_set_len(pi, offset - saved_offset);

    return offset;
}