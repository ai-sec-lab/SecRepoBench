static ndpi_int_stun_t ndpi_int_check_stun(struct ndpi_detection_module_struct *ndpi_struct,
					   struct ndpi_flow_struct *flow,
					   const u_int8_t * payload,
					   const u_int16_t payload_length) {
  u_int16_t msg_type, messagelength;
  struct stun_packet_header *h = (struct stun_packet_header*)payload;
  int rc;
  
  /* STUN over TCP does not look good */
  if(flow->packet.tcp)
    return(NDPI_IS_NOT_STUN);

  /* No need to do ntohl() with 0xFFFFFFFF */
  if(flow->packet.iph && (flow->packet.iph->daddr == 0xFFFFFFFF /* 255.255.255.255 */)) {
    NDPI_EXCLUDE_PROTO(ndpi_struct, flow);
    return(NDPI_IS_NOT_STUN);;
  }

  if(payload_length >= 512) {
    return(NDPI_IS_NOT_STUN);
  } else if(payload_length < sizeof(struct stun_packet_header)) {
    /* This looks like an invalid packet */

    if(flow->protos.stun_ssl.stun.num_udp_pkts > 0) {
      flow->guessed_host_protocol_id = NDPI_PROTOCOL_WHATSAPP_CALL;
      return(NDPI_IS_STUN);
    } else
      return(NDPI_IS_NOT_STUN);
  }

  if((strncmp((const char*)payload, (const char*)"RSP/", 4) == 0)
     && (strncmp((const char*)&payload[7], (const char*)" STUN_", 6) == 0)) {
    NDPI_LOG_INFO(ndpi_struct, "found stun\n");
    goto udp_stun_found;
  }

  msg_type = ntohs(h->msg_type), messagelength = ntohs(h->msg_len);

  if(msg_type == 0)
    return(NDPI_IS_NOT_STUN);  
  
  /* https://www.iana.org/assignments/stun-parameters/stun-parameters.xhtml */
  if((msg_type & 0x3EEF) > 0x000B && msg_type != 0x0800) {
#ifdef DEBUG_STUN
    printf("[STUN] msg_type = %04X\n", msg_type);
#endif

    /*
     If we're here it's because this does not look like STUN anymore
     as this was a flow that started as STUN and turned into something
     else. Let's investigate what is that about
     */
    if(payload[0] == 0x16) {
      /* Let's check if this is DTLS used by some socials */
      struct ndpi_packet_struct *packet = &flow->packet;
      u_int16_t total_len, version = htons(*((u_int16_t*) &packet->payload[1]));

      switch (version) {
        case 0xFEFF: /* DTLS 1.0 */
        case 0xFEFD: /* DTLS 1.2 */
          total_len = ntohs(*((u_int16_t*) &packet->payload[11])) + 13;

          if(payload_length == total_len) {
            /* This is DTLS and the only protocol we know behaves like this is signal */
            flow->guessed_host_protocol_id = NDPI_PROTOCOL_SIGNAL;
            return(NDPI_IS_STUN);
          }
      }
    }

    return(NDPI_IS_NOT_STUN);
  }

#if 0
  if((flow->packet.udp->dest == htons(3480)) ||
     (flow->packet.udp->source == htons(3480))
    )
    printf("[STUN] Here we go\n");;
#endif

  if(ndpi_struct->stun_cache) {
    u_int16_t proto;
    u_int32_t key = get_stun_lru_key(flow, 0);
    int rc = ndpi_lru_find_cache(ndpi_struct->stun_cache, key, &proto,
                                 0 /* Don't remove it as it can be used for other connections */);

#ifdef DEBUG_LRU
    printf("[LRU] Searching %u\n", key);
#endif

    if(!rc) {
      key = get_stun_lru_key(flow, 1);
      rc = ndpi_lru_find_cache(ndpi_struct->stun_cache, key, &proto,
                               0 /* Don't remove it as it can be used for other connections */);

#ifdef DEBUG_LRU
      printf("[LRU] Searching %u\n", key);
#endif
    }

    if(rc) {
#ifdef DEBUG_LRU
      printf("[LRU] Cache FOUND %u / %u\n", key, proto);
#endif

      flow->guessed_host_protocol_id = proto;
      return(NDPI_IS_STUN);
    } else {
#ifdef DEBUG_LRU
      printf("[LRU] NOT FOUND %u\n", key);
#endif
    }
  } else {
#ifdef DEBUG_LRU
    printf("[LRU] NO/EMPTY CACHE\n");
#endif
  }

  if(msg_type == 0x01 /* Binding Request */) {
    flow->protos.stun_ssl.stun.num_binding_requests++;

    if(!messagelength && flow->guessed_host_protocol_id == NDPI_PROTOCOL_GOOGLE)
      flow->guessed_host_protocol_id = NDPI_PROTOCOL_HANGOUT_DUO;
    else
      flow->guessed_protocol_id = NDPI_PROTOCOL_STUN;

    if(!messagelength) {
      /* flow->protos.stun_ssl.stun.num_udp_pkts++; */
      return(NDPI_IS_NOT_STUN); /* This to keep analyzing STUN instead of giving up */
    }
  }

  if(!messagelength && flow->guessed_host_protocol_id == NDPI_PROTOCOL_UNKNOWN) {
    NDPI_EXCLUDE_PROTO(ndpi_struct, flow);
    return(NDPI_IS_NOT_STUN);
  }

  flow->protos.stun_ssl.stun.num_udp_pkts++;

  if((payload[0] == 0x80 && payload_length < 512 && ((messagelength+20) <= payload_length))) {
    flow->guessed_host_protocol_id = NDPI_PROTOCOL_WHATSAPP_CALL;
    return(NDPI_IS_STUN); /* This is WhatsApp Call */
  } else if((payload[0] == 0x90) && (((messagelength+11) == payload_length) ||
                (flow->protos.stun_ssl.stun.num_binding_requests >= 4))) {
    flow->guessed_host_protocol_id = NDPI_PROTOCOL_WHATSAPP_CALL;
    return(NDPI_IS_STUN); /* This is WhatsApp Call */
  }

  if(payload[0] != 0x80 && (messagelength + 20) > payload_length)
    return(NDPI_IS_NOT_STUN);
  else {
    switch(flow->guessed_protocol_id) {
    case NDPI_PROTOCOL_HANGOUT_DUO:
    case NDPI_PROTOCOL_MESSENGER:
    case NDPI_PROTOCOL_WHATSAPP_CALL:
      /* Don't overwrite the protocol with sub-STUN protocols */
      break;

    default:
      flow->guessed_protocol_id = NDPI_PROTOCOL_STUN;
      break;
    }
  }

  if(payload_length == (messagelength+20)) {
    if((msg_type & 0x3EEF) <= 0x000B) /* http://www.3cx.com/blog/voip-howto/stun-details/ */ {
      u_int offset = 20;

      /*
       This can either be the standard RTCP or Ms Lync RTCP that
       later will become Ms Lync RTP. In this case we need to
       be careful before deciding about the protocol before dissecting the packet

       MS Lync = Skype
       https://en.wikipedia.org/wiki/Skype_for_Business
       */

      // <MASK>

      goto udp_stun_found;
    } else if(msg_type == 0x0800) {
      flow->guessed_host_protocol_id = NDPI_PROTOCOL_WHATSAPP_CALL;
      return(NDPI_IS_STUN);
    }
  }

  if((flow->protos.stun_ssl.stun.num_udp_pkts > 0) && (msg_type <= 0x00FF)) {
    flow->guessed_host_protocol_id = NDPI_PROTOCOL_WHATSAPP_CALL;
    return(NDPI_IS_STUN);
  } else
    return(NDPI_IS_NOT_STUN);

udp_stun_found:
  flow->protos.stun_ssl.stun.num_processed_pkts++;

  struct ndpi_packet_struct *packet = &flow->packet;

#ifdef DEBUG_STUN
  printf("==>> NDPI_PROTOCOL_WHATSAPP_CALL\n");
#endif

  if(is_messenger_ip_address(ntohl(packet->iph->saddr)) || is_messenger_ip_address(ntohl(packet->iph->daddr)))      
    flow->guessed_host_protocol_id = NDPI_PROTOCOL_MESSENGER;
  else if(is_google_ip_address(ntohl(packet->iph->saddr)) || is_google_ip_address(ntohl(packet->iph->daddr)))
    flow->guessed_host_protocol_id = NDPI_PROTOCOL_HANGOUT_DUO;
  
  rc = (flow->protos.stun_ssl.stun.num_udp_pkts < MAX_NUM_STUN_PKTS) ? NDPI_IS_NOT_STUN : NDPI_IS_STUN;

  return rc;
}