u_int8_t version_len = packet->payload[s_offset];
		char version_str[256];
		u_int8_t version_str_len = 0;
		version_str[0] = 0;
#ifdef DEBUG_TLS
		printf("Client SSL [TLS version len: %u]\n", version_len);
#endif