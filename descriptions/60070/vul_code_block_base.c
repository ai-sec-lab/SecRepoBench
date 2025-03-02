static int thrift_validate_type(uint8_t message_type)
{
  return message_type < TMT_TYPE_MAX;
}

static void thrift_set_method(struct ndpi_detection_module_struct *ndpi_struct,
                              struct ndpi_flow_struct *flow,
                              char const * const method, size_t method_length)
{
  if (thrift_validate_method(method, method_length) == 0) {
    ndpi_set_risk(ndpi_struct, flow, NDPI_INVALID_CHARACTERS, "Invalid method name");
    flow->protos.thrift.method[0] = '\0';
  } else {
    strncpy(flow->protos.thrift.method, method, method_length);
  }
}