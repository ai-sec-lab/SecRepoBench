bool sanitize (hb_sanitize_context_t *c) const
  {
    TRACE_SANITIZE (this);
    return_trace (c->check_struct (this) &&
		  !hb_unsigned_mul_overflows (numRecords, sizeDeviceRecord) &&
                  min_size + numRecords * sizeDeviceRecord > numRecords * sizeDeviceRecord &&
		  sizeDeviceRecord >= DeviceRecord::min_size &&
		  c->check_range (this, get_size ()));
  }