if (thrift_validate_method(method, method_length) == 0) {
    ndpi_set_risk(ndpi_struct, flow, NDPI_INVALID_CHARACTERS, "Invalid method name");
    flow->protos.thrift.method[0] = '\0';
  } else {
    strncpy(flow->protos.thrift.method, method, method_length);
  }