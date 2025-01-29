/*
 * mail_imap.c
 *
 * Copyright (C) 2016-20 - ntop.org
 *
 * This file is part of nDPI, an open source deep packet inspection
 * library based on the OpenDPI and PACE technology by ipoque GmbH
 *
 * nDPI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * nDPI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with nDPI.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#include "ndpi_protocol_ids.h"

#define NDPI_CURRENT_PROTO NDPI_PROTOCOL_MAIL_IMAP

#include "ndpi_api.h"

/* #define IMAP_DEBUG 1*/

static void ndpi_int_mail_imap_add_connection(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{
  ndpi_set_detected_protocol(ndpi_struct, flow, NDPI_PROTOCOL_MAIL_IMAP, NDPI_PROTOCOL_UNKNOWN);
}

void ndpi_search_mail_imap_tcp(struct ndpi_detection_module_struct *ndpi_struct, struct ndpi_flow_struct *flow)
{
  struct ndpi_packet_struct *packet = &flow->packet;       
  u_int16_t i = 0;
  u_int16_t space_pos = 0;
  u_int16_t cmdstart = 0;
  u_int8_t saw_command = 0;
  /* const u_int8_t *command = 0; */

  NDPI_LOG_DBG(ndpi_struct, "search IMAP_IMAP\n");

#ifdef IMAP_DEBUG
  printf("%s() [%s]\n", __FUNCTION__, packet->payload);
#endif

  if(flow->l4.tcp.mail_imap_starttls == 2) {
    NDPI_LOG_DBG2(ndpi_struct, "starttls detected\n");
    NDPI_ADD_PROTOCOL_TO_BITMASK(flow->excluded_protocol_bitmask, NDPI_PROTOCOL_MAIL_IMAP);
    NDPI_DEL_PROTOCOL_FROM_BITMASK(flow->excluded_protocol_bitmask, NDPI_PROTOCOL_TLS);
    return;
  }

  if(packet->payload_packet_len >= 4 && ntohs(get_u_int16_t(packet->payload, packet->payload_packet_len - 2)) == 0x0d0a) {
    // the DONE command appears without a tag
    if(packet->payload_packet_len == 6 && ((packet->payload[0] == 'D' || packet->payload[0] == 'd')
					    && (packet->payload[1] == 'O' || packet->payload[1] == 'o')
					    && (packet->payload[2] == 'N' || packet->payload[2] == 'n')
					    && (packet->payload[3] == 'E' || packet->payload[3] == 'e'))) {
      flow->l4.tcp.mail_imap_stage += 1;
      saw_command = 1;
    } else {

      if(flow->l4.tcp.mail_imap_stage < 4) {
	// search for the first space character (end of the tag)
	while (i < 20 && i < packet->payload_packet_len) {
	  if(i > 0 && packet->payload[i] == ' ') {
	    space_pos = i;
	    break;
	  }
	  if(!((packet->payload[i] >= 'a' && packet->payload[i] <= 'z') ||
		(packet->payload[i] >= 'A' && packet->payload[i] <= 'Z') ||
		(packet->payload[i] >= '0' && packet->payload[i] <= '9') || packet->payload[i] == '*' || packet->payload[i] == '.')) {
	    goto imap_excluded;
	  }
	  i++;
	}
	if(space_pos == 0 || space_pos == (packet->payload_packet_len - 1)) {
	  goto imap_excluded;
	}
	// now walk over a possible mail number to the next space
	i++;
	if(i < packet->payload_packet_len && (packet->payload[i] >= '0' && packet->payload[i] <= '9')) {
	  while (i < 20 && i < packet->payload_packet_len) {
	    if(i > 0 && packet->payload[i] == ' ') {
	      space_pos = i;
	      break;
	    }
	    if(!(packet->payload[i] >= '0' && packet->payload[i] <= '9')) {
	      goto imap_excluded;
	    }
	    i++;
	  }
	  if(space_pos == 0 || space_pos == (packet->payload_packet_len - 1)) {
	    goto imap_excluded;
	  }
	}
	cmdstart = space_pos + 1;
	/* command = &(packet->payload[command_start]); */
      } else {
	cmdstart = 0;
	/* command = &(packet->payload[command_start]); */
      }

      if((cmdstart + 3) < packet->payload_packet_len) {
	if((packet->payload[cmdstart] == 'O' || packet->payload[cmdstart] == 'o')
	    && (packet->payload[cmdstart + 1] == 'K' || packet->payload[cmdstart + 1] == 'k')
	    && packet->payload[cmdstart + 2] == ' ') {
	  flow->l4.tcp.mail_imap_stage += 1;
	  if(flow->l4.tcp.mail_imap_starttls == 1)
	    flow->l4.tcp.mail_imap_starttls = 2;
	  saw_command = 1;
	} else if((packet->payload[cmdstart] == 'U' || packet->payload[cmdstart] == 'u')
		   && (packet->payload[cmdstart + 1] == 'I' || packet->payload[cmdstart + 1] == 'i')
		   && (packet->payload[cmdstart + 2] == 'D' || packet->payload[cmdstart + 2] == 'd')) {
	  flow->l4.tcp.mail_imap_stage += 1;
	  saw_command = 1;
	}
      }
      if((cmdstart + 10) < packet->payload_packet_len) {
	if((packet->payload[cmdstart] == 'C' || packet->payload[cmdstart] == 'c')
	    && (packet->payload[cmdstart + 1] == 'A' || packet->payload[cmdstart + 1] == 'a')
	    && (packet->payload[cmdstart + 2] == 'P' || packet->payload[cmdstart + 2] == 'p')
	    && (packet->payload[cmdstart + 3] == 'A' || packet->payload[cmdstart + 3] == 'a')
	    && (packet->payload[cmdstart + 4] == 'B' || packet->payload[cmdstart + 4] == 'b')
	    && (packet->payload[cmdstart + 5] == 'I' || packet->payload[cmdstart + 5] == 'i')
	    && (packet->payload[cmdstart + 6] == 'L' || packet->payload[cmdstart + 6] == 'l')
	    && (packet->payload[cmdstart + 7] == 'I' || packet->payload[cmdstart + 7] == 'i')
	    && (packet->payload[cmdstart + 8] == 'T' || packet->payload[cmdstart + 8] == 't')
	    && (packet->payload[cmdstart + 9] == 'Y' || packet->payload[cmdstart + 9] == 'y')) {
	  flow->l4.tcp.mail_imap_stage += 1;
	  saw_command = 1;
	}
      }
      if((cmdstart + 8) < packet->payload_packet_len) {
	if((packet->payload[cmdstart] == 'S' || packet->payload[cmdstart] == 's')
	    && (packet->payload[cmdstart + 1] == 'T' || packet->payload[cmdstart + 1] == 't')
	    && (packet->payload[cmdstart + 2] == 'A' || packet->payload[cmdstart + 2] == 'a')
	    && (packet->payload[cmdstart + 3] == 'R' || packet->payload[cmdstart + 3] == 'r')
	    && (packet->payload[cmdstart + 4] == 'T' || packet->payload[cmdstart + 4] == 't')
	    && (packet->payload[cmdstart + 5] == 'T' || packet->payload[cmdstart + 5] == 't')
	    && (packet->payload[cmdstart + 6] == 'L' || packet->payload[cmdstart + 6] == 'l')
	    && (packet->payload[cmdstart + 7] == 'S' || packet->payload[cmdstart + 7] == 's')) {
        flow->l4.tcp.mail_imap_stage += 1;
        flow->l4.tcp.mail_imap_starttls = 1;
        flow->detected_protocol_stack[0] = NDPI_PROTOCOL_MAIL_IMAPS;
        saw_command = 1;
	}
      }
      if((cmdstart + 5) < packet->payload_packet_len) {
	if((packet->payload[cmdstart] == 'L' || packet->payload[cmdstart] == 'l')
	    && (packet->payload[cmdstart + 1] == 'O' || packet->payload[cmdstart + 1] == 'o')
	    && (packet->payload[cmdstart + 2] == 'G' || packet->payload[cmdstart + 2] == 'g')
	    && (packet->payload[cmdstart + 3] == 'I' || packet->payload[cmdstart + 3] == 'i')
	    && (packet->payload[cmdstart + 4] == 'N' || packet->payload[cmdstart + 4] == 'n')) {
	  /* xxxx LOGIN "username" "password" */
	  char str[256], *item;
	  u_int len = packet->payload_packet_len > sizeof(str) ? sizeof(str) : packet->payload_packet_len;
	  
	  strncpy(str, (const char*)packet->payload, len);
	  str[len] = '\0';

	  item = strchr(str, '"');
	  if(item) {
	    char *column;
	    
	    item++;
	    column = strchr(item, '"');

	    if(column) {
	      column[0] = '\0';
	      snprintf(flow->protos.ftp_imap_pop_smtp.username,
		       sizeof(flow->protos.ftp_imap_pop_smtp.username),
		       "%s", item);

	      column = strchr(&column[1], '"');
	      if(column) {
		item = &column[1];
		column = strchr(item, '"');

		if(column) {
		  column[0] = '\0';
		  snprintf(flow->protos.ftp_imap_pop_smtp.password,
			   sizeof(flow->protos.ftp_imap_pop_smtp.password),
			   "%s", item);
		}
	      }
	    }
	  }
	  
	  flow->l4.tcp.mail_imap_stage += 1;
	  saw_command = 1;
	} else if((packet->payload[cmdstart] == 'F' || packet->payload[cmdstart] == 'f')
		   && (packet->payload[cmdstart + 1] == 'E' || packet->payload[cmdstart + 1] == 'e')
		   && (packet->payload[cmdstart + 2] == 'T' || packet->payload[cmdstart + 2] == 't')
		   && (packet->payload[cmdstart + 3] == 'C' || packet->payload[cmdstart + 3] == 'c')
		   && (packet->payload[cmdstart + 4] == 'H' || packet->payload[cmdstart + 4] == 'h')) {
	  flow->l4.tcp.mail_imap_stage += 1;
	  saw_command = 1;
	} else if((packet->payload[cmdstart] == 'F' || packet->payload[cmdstart] == 'f')
		   && (packet->payload[cmdstart + 1] == 'L' || packet->payload[cmdstart + 1] == 'l')
		   && (packet->payload[cmdstart + 2] == 'A' || packet->payload[cmdstart + 2] == 'a')
		   && (packet->payload[cmdstart + 3] == 'G' || packet->payload[cmdstart + 3] == 'g')
		   && (packet->payload[cmdstart + 4] == 'S' || packet->payload[cmdstart + 4] == 's')) {
	  flow->l4.tcp.mail_imap_stage += 1;
	  saw_command = 1;
	} else if((packet->payload[cmdstart] == 'C' || packet->payload[cmdstart] == 'c')
		   && (packet->payload[cmdstart + 1] == 'H' || packet->payload[cmdstart + 1] == 'h')
		   && (packet->payload[cmdstart + 2] == 'E' || packet->payload[cmdstart + 2] == 'e')
		   && (packet->payload[cmdstart + 3] == 'C' || packet->payload[cmdstart + 3] == 'c')
		   && (packet->payload[cmdstart + 4] == 'K' || packet->payload[cmdstart + 4] == 'k')) {
	  flow->l4.tcp.mail_imap_stage += 1;
	  saw_command = 1;
	} else if((packet->payload[cmdstart] == 'S' || packet->payload[cmdstart] == 's')
		   && (packet->payload[cmdstart + 1] == 'T' || packet->payload[cmdstart + 1] == 't')
		   && (packet->payload[cmdstart + 2] == 'O' || packet->payload[cmdstart + 2] == 'o')
		   && (packet->payload[cmdstart + 3] == 'R' || packet->payload[cmdstart + 3] == 'r')
		   && (packet->payload[cmdstart + 4] == 'E' || packet->payload[cmdstart + 4] == 'e')) {
	  flow->l4.tcp.mail_imap_stage += 1;
	  saw_command = 1;
	}
      }
      if((cmdstart + 12) < packet->payload_packet_len) {
	if((packet->payload[cmdstart] == 'A' || packet->payload[cmdstart] == 'a')
	    && (packet->payload[cmdstart + 1] == 'U' || packet->payload[cmdstart + 1] == 'u')
	    && (packet->payload[cmdstart + 2] == 'T' || packet->payload[cmdstart + 2] == 't')
	    && (packet->payload[cmdstart + 3] == 'H' || packet->payload[cmdstart + 3] == 'h')
	    && (packet->payload[cmdstart + 4] == 'E' || packet->payload[cmdstart + 4] == 'e')
	    && (packet->payload[cmdstart + 5] == 'N' || packet->payload[cmdstart + 5] == 'n')
	    && (packet->payload[cmdstart + 6] == 'T' || packet->payload[cmdstart + 6] == 't')
	    && (packet->payload[cmdstart + 7] == 'I' || packet->payload[cmdstart + 7] == 'i')
	    && (packet->payload[cmdstart + 8] == 'C' || packet->payload[cmdstart + 8] == 'c')
	    && (packet->payload[cmdstart + 9] == 'A' || packet->payload[cmdstart + 9] == 'a')
	    && (packet->payload[cmdstart + 10] == 'T' || packet->payload[cmdstart + 10] == 't')
	    && (packet->payload[cmdstart + 11] == 'E' || packet->payload[cmdstart + 11] == 'e')) {
	  flow->l4.tcp.mail_imap_stage += 1;
	  saw_command = 1;
	}
      }
      if((cmdstart + 9) < packet->payload_packet_len) {
	if((packet->payload[cmdstart] == 'N' || packet->payload[cmdstart] == 'n')
	    && (packet->payload[cmdstart + 1] == 'A' || packet->payload[cmdstart + 1] == 'a')
	    && (packet->payload[cmdstart + 2] == 'M' || packet->payload[cmdstart + 2] == 'm')
	    && (packet->payload[cmdstart + 3] == 'E' || packet->payload[cmdstart + 3] == 'e')
	    && (packet->payload[cmdstart + 4] == 'S' || packet->payload[cmdstart + 4] == 's')
	    && (packet->payload[cmdstart + 5] == 'P' || packet->payload[cmdstart + 5] == 'p')
	    && (packet->payload[cmdstart + 6] == 'A' || packet->payload[cmdstart + 6] == 'a')
	    && (packet->payload[cmdstart + 7] == 'C' || packet->payload[cmdstart + 7] == 'c')
	    && (packet->payload[cmdstart + 8] == 'E' || packet->payload[cmdstart + 8] == 'e')) {
	  flow->l4.tcp.mail_imap_stage += 1;
	  saw_command = 1;
	}
      }
      if((cmdstart + 4) < packet->payload_packet_len) {
	if((packet->payload[cmdstart] == 'L' || packet->payload[cmdstart] == 'l')
	    && (packet->payload[cmdstart + 1] == 'S' || packet->payload[cmdstart + 1] == 's')
	    && (packet->payload[cmdstart + 2] == 'U' || packet->payload[cmdstart + 2] == 'u')
	    && (packet->payload[cmdstart + 3] == 'B' || packet->payload[cmdstart + 3] == 'b')) {
	  flow->l4.tcp.mail_imap_stage += 1;
	  saw_command = 1;
	} else if((packet->payload[cmdstart] == 'L' || packet->payload[cmdstart] == 'l')
		   && (packet->payload[cmdstart + 1] == 'I' || packet->payload[cmdstart + 1] == 'i')
		   && (packet->payload[cmdstart + 2] == 'S' || packet->payload[cmdstart + 2] == 's')
		   && (packet->payload[cmdstart + 3] == 'T' || packet->payload[cmdstart + 3] == 't')) {
	  flow->l4.tcp.mail_imap_stage += 1;
	  saw_command = 1;
	} else if((packet->payload[cmdstart] == 'N' || packet->payload[cmdstart] == 'n')
		   && (packet->payload[cmdstart + 1] == 'O' || packet->payload[cmdstart + 1] == 'o')
		   && (packet->payload[cmdstart + 2] == 'O' || packet->payload[cmdstart + 2] == 'o')
		   && (packet->payload[cmdstart + 3] == 'P' || packet->payload[cmdstart + 3] == 'p')) {
	  flow->l4.tcp.mail_imap_stage += 1;
	  saw_command = 1;
	} else if((packet->payload[cmdstart] == 'I' || packet->payload[cmdstart] == 'i')
		   && (packet->payload[cmdstart + 1] == 'D' || packet->payload[cmdstart + 1] == 'd')
		   && (packet->payload[cmdstart + 2] == 'L' || packet->payload[cmdstart + 2] == 'l')
		   && (packet->payload[cmdstart + 3] == 'E' || packet->payload[cmdstart + 3] == 'e')) {
	  flow->l4.tcp.mail_imap_stage += 1;
	  saw_command = 1;
	}
      }
      if((cmdstart + 6) < packet->payload_packet_len) {
	if((packet->payload[cmdstart] == 'S' || packet->payload[cmdstart] == 's')
	    && (packet->payload[cmdstart + 1] == 'E' || packet->payload[cmdstart + 1] == 'e')
	    && (packet->payload[cmdstart + 2] == 'L' || packet->payload[cmdstart + 2] == 'l')
	    && (packet->payload[cmdstart + 3] == 'E' || packet->payload[cmdstart + 3] == 'e')
	    && (packet->payload[cmdstart + 4] == 'C' || packet->payload[cmdstart + 4] == 'c')
	    && (packet->payload[cmdstart + 5] == 'T' || packet->payload[cmdstart + 5] == 't')) {
	  flow->l4.tcp.mail_imap_stage += 1;
	  saw_command = 1;
	} else if((packet->payload[cmdstart] == 'E' || packet->payload[cmdstart] == 'e')
		   && (packet->payload[cmdstart + 1] == 'X' || packet->payload[cmdstart + 1] == 'x')
		   && (packet->payload[cmdstart + 2] == 'I' || packet->payload[cmdstart + 2] == 'i')
		   && (packet->payload[cmdstart + 3] == 'S' || packet->payload[cmdstart + 3] == 's')
		   && (packet->payload[cmdstart + 4] == 'T' || packet->payload[cmdstart + 4] == 't')
		   && (packet->payload[cmdstart + 5] == 'S' || packet->payload[cmdstart + 5] == 's')) {
	  flow->l4.tcp.mail_imap_stage += 1;
	  saw_command = 1;
	} else if((packet->payload[cmdstart] == 'A' || packet->payload[cmdstart] == 'a')
		   && (packet->payload[cmdstart + 1] == 'P' || packet->payload[cmdstart + 1] == 'p')
		   && (packet->payload[cmdstart + 2] == 'P' || packet->payload[cmdstart + 2] == 'p')
		   && (packet->payload[cmdstart + 3] == 'E' || packet->payload[cmdstart + 3] == 'e')
		   && (packet->payload[cmdstart + 4] == 'N' || packet->payload[cmdstart + 4] == 'n')
		   && (packet->payload[cmdstart + 5] == 'D' || packet->payload[cmdstart + 5] == 'd')) {
	  flow->l4.tcp.mail_imap_stage += 1;
	  saw_command = 1;
	}
      }

    }

    if(saw_command == 1) {
      if((flow->l4.tcp.mail_imap_stage == 3)
	 || (flow->l4.tcp.mail_imap_stage == 5)
	 || (flow->l4.tcp.mail_imap_stage == 7)
	 ) {
	if((flow->protos.ftp_imap_pop_smtp.username[0] != '\0')
	   || (flow->l4.tcp.mail_imap_stage >= 7)) {
	  NDPI_LOG_INFO(ndpi_struct, "found MAIL_IMAP\n");
	  ndpi_int_mail_imap_add_connection(ndpi_struct, flow);
	}
	
	return;
      }
    }
  }

  if(packet->payload_packet_len > 1 && packet->payload[packet->payload_packet_len - 1] == ' ') {
    NDPI_LOG_DBG2(ndpi_struct,
	     "maybe a split imap command -> need next packet and imap_stage is set to 4.\n");
    flow->l4.tcp.mail_imap_stage = 4;
    return;
  }

 imap_excluded:

  // skip over possible authentication hashes etc. that cannot be identified as imap commands or responses
  // if the packet count is low enough and at least one command or response was seen before
  if((packet->payload_packet_len >= 2 && ntohs(get_u_int16_t(packet->payload, packet->payload_packet_len - 2)) == 0x0d0a)
      && flow->packet_counter < 6 && flow->l4.tcp.mail_imap_stage >= 1) {
    NDPI_LOG_DBG2(ndpi_struct,
	     "no imap command or response but packet count < 6 and imap stage >= 1 -> skip\n");
    return;
  }

  NDPI_EXCLUDE_PROTO(ndpi_struct, flow);
}


void init_mail_imap_dissector(struct ndpi_detection_module_struct *ndpi_struct, u_int32_t *id, NDPI_PROTOCOL_BITMASK *detection_bitmask)
{
  ndpi_set_bitmask_protocol_detection("MAIL_IMAP", ndpi_struct, detection_bitmask, *id,
				      NDPI_PROTOCOL_MAIL_IMAP,
				      ndpi_search_mail_imap_tcp,
				      NDPI_SELECTION_BITMASK_PROTOCOL_V4_V6_TCP_WITH_PAYLOAD_WITHOUT_RETRANSMISSION,
				      SAVE_DETECTION_BITMASK_AS_UNKNOWN,
				      ADD_TO_DETECTION_BITMASK);

  *id += 1;
}
