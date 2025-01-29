if(packet->payload[2] == 0x43) {
	    if(packet->payload[5] == 0xff &&
	       packet->payload[6] == 0x00 &&
	       packet->payload[7] == 0x01 &&
	       packet->payload[8] == 0x00 &&
	       packet->payload[9] == 0x00 &&
	       packet->payload[10] == 0x00 &&
	       packet->payload[11] == 0x00 &&
	       packet->payload[12] == 0x00 &&
	       packet->payload[13] == 0x00 &&
	       packet->payload[14] == 0x00) {

	    /* increase count pkt ppstream over udp */
	    flow->l4.udp.ppstream_stage++;
	    
	    ndpi_int_ppstream_add_connection(ndpi_struct, flow);
	    return;
	    }       
	  }
	  /* check 44 */
	  else if(packet->payload[2] == 0x44) {
	    /** b1 71 **/
	    if(packet->payload[3] == 0xb1 && packet->payload[4] == 0x71) {
	      if(packet->payload[13] == 0x00 &&
		 packet->payload[14] == 0x00 &&
		 packet->payload[15] == 0x01 &&
		 packet->payload[16] == 0x00) {
		/* 02 03 04 05 */
		if(packet->payload[17] == 0x02 ||
		   packet->payload[17] == 0x03 ||
		   packet->payload[17] == 0x04 ||
		   packet->payload[17] == 0x05) {
		  if(packet->payload[18] == 0x00 &&
		     packet->payload[19] == 0x00 &&
		     packet->payload[20] == 0x00) {

		    /* increase count pkt ppstream over udp */
		    flow->l4.udp.ppstream_stage++;

		    ndpi_int_ppstream_add_connection(ndpi_struct, flow);
		    return;
		  }
		}
		/* ff */
		else if(packet->payload[17] == 0xff) {
		  if(packet->payload[18] == 0xff &&
		     packet->payload[19] == 0xff &&
		     packet->payload[20] == 0xff) {

		    /* increase count pkt ppstream over udp */
		    flow->l4.udp.ppstream_stage++;
		  
		    ndpi_int_ppstream_add_connection(ndpi_struct, flow);
		    return;
		  }
		}
	      }
	    }
	    /** 73 17 **/
	    else if(packet->payload[3] == 0x73 && packet->payload[4] == 0x17) {
	      if(packet->payload[5] == 0x00 &&
		 packet->payload[6] == 0x00 &&
		 packet->payload[7] == 0x00 &&
		 packet->payload[8] == 0x00 &&
		 packet->payload[14] == 0x00 &&
		 packet->payload[15] == 0x00 &&
		 packet->payload[16] == 0x00 &&
		 packet->payload[17] == 0x00 &&
		 packet->payload[18] == 0x00 &&
		 packet->payload[19] == 0x00 &&
		 packet->payload[20] == 0x00) {

		/* increase count pkt ppstream over udp */
		flow->l4.udp.ppstream_stage++;

		ndpi_int_ppstream_add_connection(ndpi_struct, flow);
		return;
	      }
	    }
	    /** 74 71 **/
	    else if(packet->payload[3] == 0x74 && packet->payload[4] == 0x71 && packet->payload_packet_len == 113) {
	      /* check "PPStream" string in hex */
	      if(packet->payload[94] == 0x50 &&
		 packet->payload[95] == 0x50 &&
		 packet->payload[96] == 0x53 &&
		 packet->payload[97] == 0x74 &&
		 packet->payload[98] == 0x72 &&
		 packet->payload[99] == 0x65 &&
		 packet->payload[100] == 0x61 &&
		 packet->payload[101] == 0x6d) {

		/* increase count pkt ppstream over udp */
		flow->l4.udp.ppstream_stage++;
	      
		ndpi_int_ppstream_add_connection(ndpi_struct, flow);
		return;
	      }
	    }
	  }
	  /** check 55 (1) **/
	  else if(packet->payload[2] == 0x55 && (packet->payload[13] == 0x1b &&
						 packet->payload[14] == 0xa0 &&
						 packet->payload[15] == 0x00 &&
						 packet->payload[16] == 0x00 &&
						 packet->payload[17] == 0x00 &&
						 packet->payload[18] == 0x00 &&
						 packet->payload[19] == 0x00 &&
						 packet->payload[20] == 0x00 )) {

	    /* increase count pkt ppstream over udp */
	    flow->l4.udp.ppstream_stage++;

	    ndpi_int_ppstream_add_connection(ndpi_struct, flow);
	    return;
	  }
	  /** check 55 (2) **/
	  else if(packet->payload[2] == 0x55 && packet->payload[1] == 0x00 &&
		  (packet->payload[5] == 0x00 &&
		   packet->payload[6] == 0x00 &&
		   packet->payload[7] == 0x00 &&
		   packet->payload[8] == 0x00 &&
		   packet->payload[14] == 0x00 &&
		   packet->payload[15] == 0x00 &&
		   packet->payload[16] == 0x00 &&
		   packet->payload[17] == 0x00 &&
		   packet->payload[18] == 0x00 &&
		   packet->payload[19] == 0x00 &&
		   packet->payload[20] == 0x00 )) {

	    /* increase count pkt ppstream over udp */
	    flow->l4.udp.ppstream_stage++;
	  
	    ndpi_int_ppstream_add_connection(ndpi_struct, flow);
	    return;
	  }