static void thrift_set_method(struct ndpi_detection_module_struct *ndpi_struct,
                              struct ndpi_flow_struct *flow,
                              char const * const method, size_t methodlen)
{
  // Copy the method name into the flow's method field if the method is valid.
  // If the method is invalid, set a risk flag and clear the flow's method field.
  // <MASK>
}