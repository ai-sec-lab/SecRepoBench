static void ndpi_reset_packet_line_info(struct ndpi_packet_struct *pkt) {
  pkt->parsed_lines = 0, pkt->empty_line_position_set = 0, pkt->host_line.ptr = NULL,
    pkt->host_line.len = 0, pkt->referer_line.ptr = NULL, pkt->referer_line.len = 0,
    pkt->content_line.ptr = NULL, pkt->content_line.len = 0, pkt->accept_line.ptr = NULL,
    pkt->accept_line.len = 0, pkt->user_agent_line.ptr = NULL, pkt->user_agent_line.len = 0,
    pkt->http_url_name.ptr = NULL, pkt->http_url_name.len = 0, pkt->http_encoding.ptr = NULL,
    pkt->http_encoding.len = 0, pkt->http_transfer_encoding.ptr = NULL, pkt->http_transfer_encoding.len = 0,
    pkt->http_contentlen.ptr = NULL, pkt->http_contentlen.len = 0, pkt->content_disposition_line.ptr = NULL,
    pkt->content_disposition_line.len = 0, pkt->http_cookie.ptr = NULL,
    pkt->http_cookie.len = 0, pkt->http_origin.len = 0, pkt->http_origin.ptr = NULL,
    pkt->http_x_session_type.ptr = NULL, pkt->http_x_session_type.len = 0, pkt->server_line.ptr = NULL,
    pkt->server_line.len = 0, pkt->http_method.ptr = NULL, pkt->http_method.len = 0,
    pkt->http_response.ptr = NULL, pkt->http_response.len = 0, pkt->http_num_headers = 0,
    pkt->forwarded_line.ptr = NULL, pkt->forwarded_line.len = 0;
}