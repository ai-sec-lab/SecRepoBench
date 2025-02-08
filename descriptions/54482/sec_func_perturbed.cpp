bool sanitize (hb_sanitize_context_t *context) const
  {
    TRACE_SANITIZE (this);
    return_trace (context->check_struct (this) &&
		  !hb_unsigned_mul_overflows (numRecords, sizeDeviceRecord) &&
                  min_size + numRecords * sizeDeviceRecord > numRecords * sizeDeviceRecord &&
		  sizeDeviceRecord >= DeviceRecord::min_size &&
		  context->check_range (this, get_size ()));
  }