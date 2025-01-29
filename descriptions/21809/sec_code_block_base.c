if(flow->guessed_host_protocol_id == NDPI_PROTOCOL_UNKNOWN)
      flow->guessed_host_protocol_id = NDPI_PROTOCOL_WIREGUARD;
    
    if (flow->l4.udp.wireguard_stage == 0) {
      flow->l4.udp.wireguard_stage = 3 + packet->packet_direction;
      flow->l4.udp.wireguard_peer_index[packet->packet_direction] = receiver_index;
      /* need more packets before deciding */
    } else if (flow->l4.udp.wireguard_stage == 4 - packet->packet_direction) {
      flow->l4.udp.wireguard_peer_index[packet->packet_direction] = receiver_index;
      flow->l4.udp.wireguard_stage = 5;
      /* need more packets before deciding */
    } else if (flow->l4.udp.wireguard_stage == 5) {
      if (receiver_index == flow->l4.udp.wireguard_peer_index[packet->packet_direction]) {
        ndpi_set_detected_protocol(ndpi_struct, flow, NDPI_PROTOCOL_WIREGUARD, NDPI_PROTOCOL_UNKNOWN);
      } else {
        NDPI_EXCLUDE_PROTO(ndpi_struct, flow);
      }
    }
    /* need more packets before deciding */